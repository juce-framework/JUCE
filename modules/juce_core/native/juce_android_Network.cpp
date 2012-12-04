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

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "()V") \
 METHOD (toString, "toString", "()Ljava/lang/String;") \

DECLARE_JNI_CLASS (StringBuffer, "java/lang/StringBuffer");
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (release, "release", "()V") \
 METHOD (read, "read", "([BI)I") \
 METHOD (getPosition, "getPosition", "()J") \
 METHOD (getTotalLength, "getTotalLength", "()J") \
 METHOD (isExhausted, "isExhausted", "()Z") \
 METHOD (setPosition, "setPosition", "(J)Z") \

DECLARE_JNI_CLASS (HTTPStream, JUCE_ANDROID_ACTIVITY_CLASSPATH "$HTTPStream");
#undef JNI_CLASS_MEMBERS


//==============================================================================
void MACAddress::findAllAddresses (Array<MACAddress>& result)
{
    // TODO
}


bool Process::openEmailWithAttachments (const String& targetEmailAddress,
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

        LocalRef<jobject> responseHeaderBuffer (env->NewObject (StringBuffer, StringBuffer.constructor));

        stream = GlobalRef (env->CallStaticObjectMethod (JuceAppActivity,
                                                         JuceAppActivity.createHTTPStream,
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
                                                                                  StringBuffer.toString));
                headerLines.addLines (juceString (env, headersString));
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
            stream.callVoidMethod (HTTPStream.release);
    }

    //==============================================================================
    bool isExhausted()                  { return stream != nullptr && stream.callBooleanMethod (HTTPStream.isExhausted); }
    int64 getTotalLength()              { return stream != nullptr ? stream.callLongMethod (HTTPStream.getTotalLength) : 0; }
    int64 getPosition()                 { return stream != nullptr ? stream.callLongMethod (HTTPStream.getPosition) : 0; }
    bool setPosition (int64 wantedPos)  { return stream != nullptr && stream.callBooleanMethod (HTTPStream.setPosition, (jlong) wantedPos); }

    int read (void* buffer, int bytesToRead)
    {
        jassert (buffer != nullptr && bytesToRead >= 0);

        if (stream == nullptr)
            return 0;

        JNIEnv* env = getEnv();

        jbyteArray javaArray = env->NewByteArray (bytesToRead);

        int numBytes = stream.callIntMethod (HTTPStream.read, javaArray, (jint) bytesToRead);

        if (numBytes > 0)
            env->GetByteArrayRegion (javaArray, 0, numBytes, static_cast <jbyte*> (buffer));

        env->DeleteLocalRef (javaArray);
        return numBytes;
    }

    //==============================================================================
    GlobalRef stream;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebInputStream)
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
