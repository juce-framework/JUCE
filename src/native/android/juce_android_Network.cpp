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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
void MACAddress::findAllAddresses (Array<MACAddress>& result)
{
    // TODO
}


bool PlatformUtilities::launchEmailWithAttachments (const String& targetEmailAddress,
                                                    const String& emailSubject,
                                                    const String& bodyText,
                                                    const StringArray& filesToAttach)
{
    // TODO

    return false;
}


//==============================================================================
class WebInputStream  : public InputStream
{
public:
    //==============================================================================
    WebInputStream (String address, bool isPost, const MemoryBlock& postData,
                    URL::OpenStreamProgressCallback* progressCallback, void* progressCallbackContext,
                    const String& headers, int timeOutMs, StringPairArray* responseHeaders)
    {
        if (! address.contains ("://"))
            address = "http://" + address;

        JNIEnv* env = getEnv();

        jbyteArray postDataArray = 0;

        if (postData.getSize() > 0)
        {
            postDataArray = env->NewByteArray (postData.getSize());
            env->SetByteArrayRegion (postDataArray, 0, postData.getSize(), (const jbyte*) postData.getData());
        }

        LocalRef<jobject> responseHeaderBuffer (env->NewObject (android.stringBufferClass, android.stringBufferConstructor));

        stream = GlobalRef (env->CallStaticObjectMethod (android.activityClass,
                                                         android.createHTTPStream,
                                                         javaString (address).get(),
                                                         (jboolean) isPost,
                                                         postDataArray,
                                                         javaString (headers).get(),
                                                         (jint) timeOutMs,
                                                         responseHeaderBuffer.get()));

        if (postDataArray != 0)
            env->DeleteLocalRef (postDataArray);

        if (stream != 0)
        {
            StringArray headerLines;

            {
                LocalRef<jstring> headersString ((jstring) env->CallObjectMethod (responseHeaderBuffer.get(),
                                                                                  android.stringBufferToString));
                headerLines.addLines (juceString (headersString));
            }

            if (responseHeaders != 0)
            {
                for (int i = 0; i < headerLines.size(); ++i)
                {
                    const String& header = headerLines[i];
                    const String key (header.upToFirstOccurrenceOf (": ", false, false));
                    const String value (header.fromFirstOccurrenceOf (": ", false, false));
                    const String previousValue ((*responseHeaders) [key]);

                    responseHeaders->set (key, previousValue.isEmpty() ? value : (previousValue + "," + value));
                }
            }
        }
    }

    ~WebInputStream()
    {
        if (stream != 0)
            stream.callVoidMethod (android.httpStreamRelease);
    }

    //==============================================================================
    bool isExhausted()                  { return stream != nullptr && stream.callBooleanMethod (android.isExhausted); }
    int64 getTotalLength()              { return stream != nullptr ? stream.callLongMethod (android.getTotalLength) : 0; }
    int64 getPosition()                 { return stream != nullptr ? stream.callLongMethod (android.getPosition) : 0; }
    bool setPosition (int64 wantedPos)  { return stream != nullptr && stream.callBooleanMethod (android.setPosition, (jlong) wantedPos); }

    int read (void* buffer, int bytesToRead)
    {
        if (stream == nullptr)
            return 0;

        JNIEnv* env = getEnv();

        jbyteArray javaArray = env->NewByteArray (bytesToRead);

        int numBytes = stream.callIntMethod (android.httpStreamRead, javaArray, (jint) bytesToRead);

        if (numBytes > 0)
            env->GetByteArrayRegion (javaArray, 0, numBytes, static_cast <jbyte*> (buffer));

        env->DeleteLocalRef (javaArray);
        return numBytes;
    }

    //==============================================================================
    GlobalRef stream;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebInputStream);
};

InputStream* URL::createNativeStream (const String& address, bool isPost, const MemoryBlock& postData,
                                      OpenStreamProgressCallback* progressCallback, void* progressCallbackContext,
                                      const String& headers, const int timeOutMs, StringPairArray* responseHeaders)
{
    ScopedPointer <WebInputStream> wi (new WebInputStream (address, isPost, postData,
                                                           progressCallback, progressCallbackContext,
                                                           headers, timeOutMs, responseHeaders));

    return wi->stream != 0 ? wi.release() : nullptr;
}

#endif
