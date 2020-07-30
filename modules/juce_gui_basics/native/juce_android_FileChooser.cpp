/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class FileChooser::Native     : public FileChooser::Pimpl
{
public:
    //==============================================================================
    Native (FileChooser& fileChooser, int flags)    : owner (fileChooser)
    {
        if (currentFileChooser == nullptr)
        {
            currentFileChooser = this;
            auto* env = getEnv();

            auto sdkVersion         = getAndroidSDKVersion();
            auto saveMode           = ((flags & FileBrowserComponent::saveMode) != 0);
            auto selectsDirectories = ((flags & FileBrowserComponent::canSelectDirectories) != 0);

            // You cannot save a directory
            jassert (! (saveMode && selectsDirectories));

            if (sdkVersion < 19)
            {
                // native save dialogs are only supported in Android versions >= 19
                jassert (! saveMode);
                saveMode = false;
            }

            if (sdkVersion < 21)
            {
                // native directory chooser dialogs are only supported in Android versions >= 21
                jassert (! selectsDirectories);
                selectsDirectories = false;
            }

            const char* action = (selectsDirectories ? "android.intent.action.OPEN_DOCUMENT_TREE"
                                                     : (saveMode ? "android.intent.action.CREATE_DOCUMENT"
                                                     : (sdkVersion >= 19 ? "android.intent.action.OPEN_DOCUMENT"
                                                     : "android.intent.action.GET_CONTENT")));


            intent = GlobalRef (LocalRef<jobject> (env->NewObject (AndroidIntent, AndroidIntent.constructWithString,
                                                                   javaString (action).get())));

            if (owner.startingFile != File())
            {
                if (saveMode && (! owner.startingFile.isDirectory()))
                    env->CallObjectMethod (intent.get(), AndroidIntent.putExtraString,
                                           javaString ("android.intent.extra.TITLE").get(),
                                           javaString (owner.startingFile.getFileName()).get());


                URL url (owner.startingFile);
                LocalRef<jobject> uri (env->CallStaticObjectMethod (AndroidUri, AndroidUri.parse,
                                                                    javaString (url.toString (true)).get()));

                if (uri)
                    env->CallObjectMethod (intent.get(), AndroidIntent.putExtraParcelable,
                                           javaString ("android.provider.extra.INITIAL_URI").get(),
                                           uri.get());
            }


            if (! selectsDirectories)
            {
                env->CallObjectMethod (intent.get(), AndroidIntent.addCategory,
                                       javaString ("android.intent.category.OPENABLE").get());

                auto mimeTypes = convertFiltersToMimeTypes (owner.filters);

                if (mimeTypes.size() == 1)
                {
                    env->CallObjectMethod (intent.get(), AndroidIntent.setType, javaString (mimeTypes[0]).get());
                }
                else
                {
                    String mimeGroup = "*";

                    if (mimeTypes.size() > 0)
                    {
                        mimeGroup = mimeTypes[0].upToFirstOccurrenceOf ("/", false, false);
                        auto allMimeTypesHaveSameGroup = true;

                        LocalRef<jobjectArray> jMimeTypes (env->NewObjectArray (mimeTypes.size(), JavaString,
                                                                                javaString("").get()));

                        for (int i = 0; i < mimeTypes.size(); ++i)
                        {
                            env->SetObjectArrayElement (jMimeTypes.get(), i, javaString (mimeTypes[i]).get());

                            if (mimeGroup != mimeTypes[i].upToFirstOccurrenceOf ("/", false, false))
                                allMimeTypesHaveSameGroup = false;
                        }

                        env->CallObjectMethod (intent.get(), AndroidIntent.putExtraStrings,
                                               javaString ("android.intent.extra.MIME_TYPES").get(),
                                               jMimeTypes.get());

                        if (! allMimeTypesHaveSameGroup)
                            mimeGroup = "*";
                    }

                    env->CallObjectMethod (intent.get(), AndroidIntent.setType, javaString (mimeGroup + "/*").get());
                }
            }
        }
        else
            jassertfalse; // there can only be a single file chooser
    }

    ~Native() override
    {
        masterReference.clear();
        currentFileChooser = nullptr;
    }

    void runModally() override
    {
        // Android does not support modal file choosers
        jassertfalse;
    }

    void launch() override
    {
        auto* env = getEnv();

        if (currentFileChooser != nullptr)
        {
            WeakReference<Native> myself (this);

            startAndroidActivityForResult (LocalRef<jobject> (env->NewLocalRef (intent.get())), /*READ_REQUEST_CODE*/ 42,
                                           [myself] (int requestCode, int resultCode, LocalRef<jobject> intentData) mutable
                                           {
                                               if (myself != nullptr)
                                                   myself->onActivityResult (requestCode, resultCode, intentData);
                                           });
        }
        else
        {
            jassertfalse; // There is already a file chooser running
        }
    }

    void onActivityResult (int /*requestCode*/, int resultCode, const LocalRef<jobject>& intentData)
    {
        currentFileChooser = nullptr;
        auto* env = getEnv();

        Array<URL> chosenURLs;

        if (resultCode == /*Activity.RESULT_OK*/ -1 && intentData != nullptr)
        {
            LocalRef<jobject> uri (env->CallObjectMethod (intentData.get(), AndroidIntent.getData));

            if (uri != nullptr)
            {
                auto jStr = (jstring) env->CallObjectMethod (uri, JavaObject.toString);

                if (jStr != nullptr)
                    chosenURLs.add (URL (juceString (env, jStr)));
            }
        }

        owner.finished (chosenURLs);
    }

    static Native* currentFileChooser;

    static StringArray convertFiltersToMimeTypes (const String& fileFilters)
    {
        StringArray result;
        auto wildcards = StringArray::fromTokens (fileFilters, ";", "");

        for (auto wildcard : wildcards)
        {
            if (wildcard.upToLastOccurrenceOf (".", false, false) == "*")
            {
                auto extension = wildcard.fromLastOccurrenceOf (".", false, false);

                result.addArray (getMimeTypesForFileExtension (extension));
            }
        }

        result.removeDuplicates (false);
        return result;
    }

private:
    JUCE_DECLARE_WEAK_REFERENCEABLE (Native)

    FileChooser& owner;
    GlobalRef intent;
};

FileChooser::Native* FileChooser::Native::currentFileChooser = nullptr;

FileChooser::Pimpl* FileChooser::showPlatformDialog (FileChooser& owner, int flags,
                                                     FilePreviewComponent*)
{
    if (FileChooser::Native::currentFileChooser == nullptr)
        return new FileChooser::Native (owner, flags);

    // there can only be one file chooser on Android at a once
    jassertfalse;
    return nullptr;
}

bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    return true;
   #endif
}

} // namespace juce
