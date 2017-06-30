/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/


//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (play,                  "play",           "()V") \
 METHOD (stop,                  "stop",           "()V") \
 METHOD (close,                 "close",          "()V") \
 METHOD (isPlaying,             "isPlaying",      "()Z") \
 METHOD (loadFile,              "loadFile",       "(Ljava/lang/String;)Z") \
 METHOD (loadURL,               "loadURL",        "(Ljava/lang/String;)Z") \
 METHOD (setCurrentPosition,    "setCurrentPosition", "(D)V") \
 METHOD (getCurrentPosition,    "getCurrentPosition", "()D") \
 METHOD (setSpeed,              "setSpeed",       "(D)V") \
 METHOD (getDuration,           "getDuration",    "()D") \
 METHOD (getVideoWidth,         "getVideoWidth",  "()I") \
 METHOD (getVideoHeight,        "getVideoHeight", "()I") \
 METHOD (setVolume,             "setVolume",      "(F)V") \
 METHOD (getVolume,             "getVolume",      "()F") \

DECLARE_JNI_CLASS (VideoView, JUCE_ANDROID_ACTIVITY_CLASSPATH "$VideoView")
#undef JNI_CLASS_MEMBERS


struct VideoComponent::Pimpl   : public Component
{
    Pimpl()
    {
    }

    ~Pimpl()
    {
        close();
    }

    Result load (const File& file)
    {
        if (isOpen() && videoView.callBooleanMethod (VideoView.loadFile, javaString (file.getFullPathName()).get()))
        {
            currentFile = file;
            return Result::ok();
        }

        return Result::fail ("Couldn't open file");
    }

    Result load (const URL& url)
    {
        if (isOpen() && videoView.callBooleanMethod (VideoView.loadFile, javaString (url.toString (true)).get()))
        {
            currentURL = url;
            return Result::ok();
        }

        return Result::fail ("Couldn't open file");
    }

    void close()
    {
        if (isOpen())
            videoView.callVoidMethod (VideoView.close);
    }

    bool isOpen() const
    {
        return videoView != nullptr;
    }

    bool isPlaying() const
    {
        return isOpen() && videoView.callBooleanMethod (VideoView.isPlaying);
    }

    void play()
    {
        if (isOpen())
            videoView.callVoidMethod (VideoView.play);
    }

    void stop()
    {
        if (isOpen())
            videoView.callVoidMethod (VideoView.stop);
    }

    void setPosition (double newPosition)
    {
        if (isOpen())
            videoView.callVoidMethod (VideoView.setCurrentPosition, (jdouble) newPosition);
    }

    double getPosition() const
    {
        if (isOpen())
            return videoView.callDoubleMethod (VideoView.getCurrentPosition);

        return 0.0;
    }

    void setSpeed (double newSpeed)
    {
        if (isOpen())
            videoView.callVoidMethod (VideoView.setSpeed, (jdouble) newSpeed);
    }

    Rectangle<int> getNativeSize() const
    {
        if (isOpen())
        {
            jint w = videoView.callIntMethod (VideoView.getVideoWidth);
            jint h = videoView.callIntMethod (VideoView.getVideoHeight);

            return Rectangle<int> (w, h);
        }

        return Rectangle<int>();
    }

    double getDuration() const
    {
        if (isOpen())
            return videoView.callDoubleMethod (VideoView.getDuration);

        return 0.0;
    }

    void setVolume (float newVolume)
    {
        if (isOpen())
            videoView.callVoidMethod (VideoView.setVolume, (jfloat) newVolume);
    }

    float getVolume() const
    {
        if (isOpen())
            return videoView.callFloatMethod (VideoView.getVolume);

        return 0.0f;
    }

    File currentFile;
    URL currentURL;

    GlobalRef videoView;
};
