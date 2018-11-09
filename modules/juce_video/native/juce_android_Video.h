/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2018 - ROLI Ltd.

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

#if __ANDROID_API__ >= 21
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getPlaybackInfo,      "getPlaybackInfo",      "()Landroid/media/session/MediaController$PlaybackInfo;") \
 METHOD (getPlaybackState,     "getPlaybackState",     "()Landroid/media/session/PlaybackState;") \
 METHOD (getTransportControls, "getTransportControls", "()Landroid/media/session/MediaController$TransportControls;") \
 METHOD (registerCallback,     "registerCallback",     "(Landroid/media/session/MediaController$Callback;)V") \
 METHOD (setVolumeTo,          "setVolumeTo",          "(II)V") \
 METHOD (unregisterCallback,   "unregisterCallback",   "(Landroid/media/session/MediaController$Callback;)V")

DECLARE_JNI_CLASS (AndroidMediaController, "android/media/session/MediaController")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";J)V") \

DECLARE_JNI_CLASS (AndroidMediaControllerCallback, JUCE_ANDROID_ACTIVITY_CLASSPATH "$MediaControllerCallback")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getAudioAttributes, "getAudioAttributes", "()Landroid/media/AudioAttributes;") \
 METHOD (getCurrentVolume,   "getCurrentVolume",   "()I") \
 METHOD (getMaxVolume,       "getMaxVolume",       "()I")

DECLARE_JNI_CLASS (AndroidMediaControllerPlaybackInfo, "android/media/session/MediaController$PlaybackInfo")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (pause,           "pause",           "()V") \
 METHOD (play,            "play",            "()V") \
 METHOD (playFromMediaId, "playFromMediaId", "(Ljava/lang/String;Landroid/os/Bundle;)V") \
 METHOD (seekTo,          "seekTo",          "(J)V") \
 METHOD (stop,            "stop",            "()V")

DECLARE_JNI_CLASS (AndroidMediaControllerTransportControls, "android/media/session/MediaController$TransportControls")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,                  "<init>",                       "()V") \
 METHOD (getCurrentPosition,           "getCurrentPosition",           "()I") \
 METHOD (getDuration,                  "getDuration",                  "()I") \
 METHOD (getPlaybackParams,            "getPlaybackParams",            "()Landroid/media/PlaybackParams;") \
 METHOD (getVideoHeight,               "getVideoHeight",               "()I") \
 METHOD (getVideoWidth,                "getVideoWidth",                "()I") \
 METHOD (isPlaying,                    "isPlaying",                    "()Z") \
 METHOD (pause,                        "pause",                        "()V") \
 METHOD (prepareAsync,                 "prepareAsync",                 "()V") \
 METHOD (release,                      "release",                      "()V") \
 METHOD (seekTo,                       "seekTo",                       "(I)V") \
 METHOD (setAudioAttributes,           "setAudioAttributes",           "(Landroid/media/AudioAttributes;)V") \
 METHOD (setDataSource,                "setDataSource",                "(Landroid/content/Context;Landroid/net/Uri;)V") \
 METHOD (setDisplay,                   "setDisplay",                   "(Landroid/view/SurfaceHolder;)V") \
 METHOD (setOnBufferingUpdateListener, "setOnBufferingUpdateListener", "(Landroid/media/MediaPlayer$OnBufferingUpdateListener;)V") \
 METHOD (setOnCompletionListener,      "setOnCompletionListener",      "(Landroid/media/MediaPlayer$OnCompletionListener;)V") \
 METHOD (setOnErrorListener,           "setOnErrorListener",           "(Landroid/media/MediaPlayer$OnErrorListener;)V") \
 METHOD (setOnInfoListener,            "setOnInfoListener",            "(Landroid/media/MediaPlayer$OnInfoListener;)V") \
 METHOD (setOnPreparedListener,        "setOnPreparedListener",        "(Landroid/media/MediaPlayer$OnPreparedListener;)V") \
 METHOD (setOnSeekCompleteListener,    "setOnSeekCompleteListener",    "(Landroid/media/MediaPlayer$OnSeekCompleteListener;)V") \
 METHOD (setPlaybackParams,            "setPlaybackParams",            "(Landroid/media/PlaybackParams;)V") \
 METHOD (setVolume,                    "setVolume",                    "(FF)V") \
 METHOD (start,                        "start",                        "()V") \
 METHOD (stop,                         "stop",                         "()V")

DECLARE_JNI_CLASS (AndroidMediaPlayer, "android/media/MediaPlayer")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor,            "<init>",                 "(Landroid/content/Context;Ljava/lang/String;)V") \
 METHOD (getController,          "getController",          "()Landroid/media/session/MediaController;") \
 METHOD (release,                "release",                "()V") \
 METHOD (setActive,              "setActive",              "(Z)V") \
 METHOD (setCallback,            "setCallback",            "(Landroid/media/session/MediaSession$Callback;)V") \
 METHOD (setFlags,               "setFlags",               "(I)V") \
 METHOD (setMediaButtonReceiver, "setMediaButtonReceiver", "(Landroid/app/PendingIntent;)V") \
 METHOD (setMetadata,            "setMetadata",            "(Landroid/media/MediaMetadata;)V") \
 METHOD (setPlaybackState,       "setPlaybackState",       "(Landroid/media/session/PlaybackState;)V") \
 METHOD (setPlaybackToLocal,     "setPlaybackToLocal",     "(Landroid/media/AudioAttributes;)V")

DECLARE_JNI_CLASS (AndroidMediaSession, "android/media/session/MediaSession")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>", "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";J)V") \

DECLARE_JNI_CLASS (AndroidMediaSessionCallback, JUCE_ANDROID_ACTIVITY_CLASSPATH "$MediaSessionCallback")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (build,       "build",   "()Landroid/media/MediaMetadata;") \
 METHOD (constructor, "<init>",  "()V") \
 METHOD (putLong,     "putLong", "(Ljava/lang/String;J)Landroid/media/MediaMetadata$Builder;")

DECLARE_JNI_CLASS (AndroidMediaMetadataBuilder, "android/media/MediaMetadata$Builder")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getSpeed, "getSpeed", "()F") \
 METHOD (setSpeed, "setSpeed", "(F)Landroid/media/PlaybackParams;")

DECLARE_JNI_CLASS (AndroidPlaybackParams, "android/media/PlaybackParams")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (getActions,       "getActions",       "()J") \
 METHOD (getErrorMessage,  "getErrorMessage",  "()Ljava/lang/CharSequence;") \
 METHOD (getPlaybackSpeed, "getPlaybackSpeed", "()F") \
 METHOD (getPosition,      "getPosition",      "()J") \
 METHOD (getState,         "getState",         "()I")

DECLARE_JNI_CLASS (AndroidPlaybackState, "android/media/session/PlaybackState")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (build,           "build",           "()Landroid/media/session/PlaybackState;") \
 METHOD (constructor,     "<init>",          "()V") \
 METHOD (setActions,      "setActions",      "(J)Landroid/media/session/PlaybackState$Builder;") \
 METHOD (setErrorMessage, "setErrorMessage", "(Ljava/lang/CharSequence;)Landroid/media/session/PlaybackState$Builder;") \
 METHOD (setState,        "setState",        "(IJF)Landroid/media/session/PlaybackState$Builder;")

DECLARE_JNI_CLASS (AndroidPlaybackStateBuilder, "android/media/session/PlaybackState$Builder")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (constructor, "<init>",     "(L" JUCE_ANDROID_ACTIVITY_CLASSPATH ";Landroid/app/Activity;J)V") \
 METHOD (setEnabled,  "setEnabled", "(Z)V")

DECLARE_JNI_CLASS (SystemVolumeObserver, JUCE_ANDROID_ACTIVITY_CLASSPATH "$SystemVolumeObserver")
#undef JNI_CLASS_MEMBERS

#endif

//==============================================================================
class MediaPlayerListener  : public AndroidInterfaceImplementer
{
public:
    struct Owner
    {
        virtual ~Owner() {}

        virtual void onPrepared (LocalRef<jobject>& mediaPlayer) = 0;
        virtual void onBufferingUpdate (LocalRef<jobject>& mediaPlayer, int progress) = 0;
        virtual void onSeekComplete (LocalRef<jobject>& mediaPlayer) = 0;
        virtual void onCompletion (LocalRef<jobject>& mediaPlayer) = 0;
        virtual bool onInfo (LocalRef<jobject>& mediaPlayer, int what, int extra) = 0;
        virtual bool onError (LocalRef<jobject>& mediaPlayer, int what, int extra) = 0;
    };

    MediaPlayerListener (Owner& ownerToUse) : owner (ownerToUse) {}

private:
    Owner& owner;

    jobject invoke (jobject proxy, jobject method, jobjectArray args) override
    {
        auto* env = getEnv();
        auto methodName = juce::juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

        int numArgs = args != nullptr ? env->GetArrayLength (args) : 0;

        if (methodName == "onPrepared" && numArgs == 1)
        {
            auto mediaPlayer = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));

            owner.onPrepared (mediaPlayer);
            return nullptr;
        }

        if (methodName == "onCompletion" && numArgs == 1)
        {
            auto mediaPlayer = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));

            owner.onCompletion (mediaPlayer);
            return nullptr;
        }

        if (methodName == "onInfo" && numArgs == 3)
        {
            auto mediaPlayer = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));
            auto what        = LocalRef<jobject> (env->GetObjectArrayElement (args, 1));
            auto extra       = LocalRef<jobject> (env->GetObjectArrayElement (args, 2));

            auto whatInt  = (int) env->CallIntMethod (what, JavaInteger.intValue);
            auto extraInt = (int) env->CallIntMethod (extra, JavaInteger.intValue);

            auto res = owner.onInfo (mediaPlayer, whatInt, extraInt);
            return env->CallStaticObjectMethod (JavaBoolean, JavaBoolean.valueOf, (jboolean) res);
        }

        if (methodName == "onError" && numArgs == 3)
        {
            auto mediaPlayer = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));
            auto what        = LocalRef<jobject> (env->GetObjectArrayElement (args, 1));
            auto extra       = LocalRef<jobject> (env->GetObjectArrayElement (args, 2));

            auto whatInt  = (int) env->CallIntMethod (what, JavaInteger.intValue);
            auto extraInt = (int) env->CallIntMethod (extra, JavaInteger.intValue);

            auto res = owner.onError (mediaPlayer, whatInt, extraInt);
            return env->CallStaticObjectMethod (JavaBoolean, JavaBoolean.valueOf, (jboolean) res);
        }

        if (methodName == "onSeekComplete" && numArgs == 1)
        {
            auto mediaPlayer = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));

            owner.onSeekComplete (mediaPlayer);
            return nullptr;
        }

        if (methodName == "onBufferingUpdate" && numArgs == 2)
        {
            auto mediaPlayer = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));

            auto progress    = LocalRef<jobject> (env->GetObjectArrayElement (args, 1));
            auto progressInt = (int) env->CallIntMethod (progress, JavaInteger.intValue);

            owner.onBufferingUpdate (mediaPlayer, progressInt);

            return nullptr;
        }

        return AndroidInterfaceImplementer::invoke (proxy, method, args);
    }
};

//==============================================================================
class AudioManagerOnAudioFocusChangeListener  : public AndroidInterfaceImplementer
{
public:
    struct Owner
    {
        virtual ~Owner() {}

        virtual void onAudioFocusChange (int changeType) = 0;
    };

    AudioManagerOnAudioFocusChangeListener (Owner& ownerToUse) : owner (ownerToUse) {}

private:
    Owner& owner;

    jobject invoke (jobject proxy, jobject method, jobjectArray args) override
    {
        auto* env = getEnv();
        auto methodName = juce::juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

        int numArgs = args != nullptr ? env->GetArrayLength (args) : 0;

        if (methodName == "onAudioFocusChange" && numArgs == 1)
        {
            auto changeType = LocalRef<jobject> (env->GetObjectArrayElement (args, 0));

            auto changeTypeInt = (int) env->CallIntMethod (changeType, JavaInteger.intValue);

            owner.onAudioFocusChange (changeTypeInt);
            return nullptr;
        }

        return AndroidInterfaceImplementer::invoke (proxy, method, args);
    }
};

//==============================================================================
struct VideoComponent::Pimpl
    : public AndroidViewComponent
#if __ANDROID_API__ >= 21
    , private AppPausedResumedListener::Owner
#endif
{
    Pimpl (VideoComponent& ownerToUse, bool)
       #if __ANDROID_API__ >= 21
        : owner (ownerToUse),
          mediaSession (*this),
          appPausedResumedListener (*this),
          appPausedResumedListenerNative (CreateJavaInterface (&appPausedResumedListener,
                                                               JUCE_ANDROID_ACTIVITY_CLASSPATH "$AppPausedResumedListener").get())
       #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
        , systemVolumeListener (*this)
       #endif
       #endif
    {
       #if __ANDROID_API__ >= 21
        setVisible (true);

        auto* env = getEnv();

        setView (LocalRef<jobject> (env->CallObjectMethod (android.activity.get(),
                                                           JuceAppActivity.createNativeSurfaceView,
                                                           reinterpret_cast<jlong> (this),
                                                           true)));

        env->CallVoidMethod (android.activity, JuceAppActivity.addAppPausedResumedListener,
                             appPausedResumedListenerNative.get(), reinterpret_cast<jlong> (this));
       #endif
    }

    ~Pimpl()
    {
       #if __ANDROID_API__ >= 21
        getEnv()->CallVoidMethod (android.activity, JuceAppActivity.removeAppPausedResumedListener,
                                  appPausedResumedListenerNative.get(), reinterpret_cast<jlong>(this));
       #endif
    }

   #if __ANDROID_API__ < 21
    // Dummy implementations for unsupported API levels.
    void loadAsync (const URL&, std::function<void (const URL&, Result)>) {}
    void close()                          {}
    bool isOpen() const noexcept          { return false; }
    bool isPlaying() const noexcept       { return false; }
    void play()                           {}
    void stop()                           {}
    void setPosition (double)             {}
    void setSpeed (double)                {}
    void setVolume (float)                {}
    float getVolume() const               { return 0.0f; }
    double getPosition() const            { return 0.0; }
    double getSpeed() const               { return 0.0; }
    Rectangle<int> getNativeSize() const  { return {}; }
    double getDuration() const            { return 0.0; }

    File currentFile;
    URL currentURL;
   #else
    void loadAsync (const URL& url, std::function<void (const URL&, Result)> callback)
    {
        close();
        wasOpen = false;

        if (url.isEmpty())
        {
            jassertfalse;
            return;
        }

        if (! url.isLocalFile())
        {
            auto granted = android.activity.callBooleanMethod (JuceAppActivity.isPermissionDeclaredInManifestString,
                                                               javaString ("android.permission.INTERNET").get()) != 0;

            if (! granted)
            {
                // In order to access videos from the Internet, the Internet permission has to be specified in
                // Android Manifest.
                jassertfalse;
                return;
            }
        }

        currentURL = url;

        jassert (callback != nullptr);

        loadFinishedCallback = std::move (callback);

        static constexpr jint visible = 0;
        getEnv()->CallVoidMethod ((jobject) getView(), AndroidView.setVisibility, visible);

        mediaSession.load (url);
    }

    void close()
    {
        if (! isOpen())
            return;

        mediaSession.closeVideo();

        static constexpr jint invisible = 4;
        getEnv()->CallVoidMethod ((jobject) getView(), AndroidView.setVisibility, invisible);
    }

    bool isOpen() const noexcept          { return mediaSession.isVideoOpen(); }
    bool isPlaying() const noexcept       { return mediaSession.isPlaying(); }

    void play()                           { mediaSession.play(); }
    void stop()                           { mediaSession.stop(); }

    void setPosition (double newPosition) { mediaSession.setPosition (newPosition); }
    double getPosition() const            { return mediaSession.getPosition(); }

    void setSpeed (double newSpeed)       { mediaSession.setSpeed (newSpeed); }
    double getSpeed() const               { return mediaSession.getSpeed(); }

    Rectangle<int> getNativeSize() const  { return mediaSession.getNativeSize(); }

    double getDuration() const            { return mediaSession.getDuration(); }

    void setVolume (float newVolume)      { mediaSession.setVolume (newVolume); }
    float getVolume() const               { return mediaSession.getVolume(); }

    File currentFile;
    URL currentURL;

private:
    //==============================================================================
    class MediaSession  : private AudioManagerOnAudioFocusChangeListener::Owner
    {
    public:
        MediaSession (Pimpl& ownerToUse)
            : owner (ownerToUse),
              sdkVersion (getEnv()->CallStaticIntMethod (JuceAppActivity, JuceAppActivity.getAndroidSDKVersion)),
              audioAttributes (getAudioAttributes()),
              nativeMediaSession (LocalRef<jobject> (getEnv()->NewObject (AndroidMediaSession,
                                                                          AndroidMediaSession.constructor,
                                                                          android.activity.get(),
                                                                          javaString ("JuceVideoMediaSession").get()))),
              mediaSessionCallback (LocalRef<jobject> (getEnv()->NewObject (AndroidMediaSessionCallback,
                                                                            AndroidMediaSessionCallback.constructor,
                                                                            android.activity.get(),
                                                                            reinterpret_cast<jlong> (this)))),
              playbackStateBuilder (LocalRef<jobject> (getEnv()->NewObject (AndroidPlaybackStateBuilder,
                                                                            AndroidPlaybackStateBuilder.constructor))),
              controller (*this, getEnv()->CallObjectMethod (nativeMediaSession,
                                                             AndroidMediaSession.getController)),
              player (*this),
              audioManager (android.activity.callObjectMethod (JuceAppActivity.getSystemService, javaString ("audio").get())),
              audioFocusChangeListener (*this),
              nativeAudioFocusChangeListener (GlobalRef (CreateJavaInterface (&audioFocusChangeListener,
                                                                              "android/media/AudioManager$OnAudioFocusChangeListener").get())),
              audioFocusRequest (createAudioFocusRequestIfNecessary (sdkVersion, audioAttributes,
                                                                     nativeAudioFocusChangeListener))
        {
            auto* env = getEnv();

            env->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setPlaybackToLocal, audioAttributes.get());
            env->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setMediaButtonReceiver, nullptr);
            env->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setCallback, mediaSessionCallback.get());
        }

        ~MediaSession()
        {
            auto* env = getEnv();

            env->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setCallback, nullptr);

            controller.stop();
            env->CallVoidMethod (nativeMediaSession, AndroidMediaSession.release);
        }

        bool isVideoOpen() const { return player.isVideoOpen(); }
        bool isPlaying() const   { return player.isPlaying(); }

        void load (const URL& url) { controller.load (url); }

        void closeVideo()
        {
            resetState();
            controller.closeVideo();
        }

        void setDisplay (jobject surfaceHolder) { player.setDisplay (surfaceHolder); }

        void play() { controller.play(); }
        void stop() { controller.stop(); }

        void setPosition (double newPosition) { controller.setPosition (newPosition); }
        double getPosition() const            { return controller.getPosition(); }

        void setSpeed (double newSpeed)
        {
            playSpeedMult = newSpeed;

            // Calling non 0.0 speed on a paused player would start it...
            if (player.isPlaying())
            {
                player.setPlaySpeed (playSpeedMult);
                updatePlaybackState();
            }
        }

        double getSpeed() const              { return controller.getPlaySpeed(); }
        Rectangle<int> getNativeSize() const { return player.getVideoNativeSize(); }
        double getDuration() const           { return player.getVideoDuration() / 1000.0; }

        void setVolume (float newVolume)
        {
           #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
            controller.setVolume (newVolume);
           #else
            player.setAudioVolume (newVolume);
           #endif
        }

        float getVolume() const
        {
           #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
            return controller.getVolume();
           #else
            return player.getAudioVolume();
           #endif
        }

        void storeState()
        {
            storedPlaybackState.clear();
            storedPlaybackState = GlobalRef (getCurrentPlaybackState());
        }

        void restoreState()
        {
            if (storedPlaybackState.get() == nullptr)
                return;

            auto* env = getEnv();

            auto pos = env->CallLongMethod (storedPlaybackState, AndroidPlaybackState.getPosition);
            setPosition (pos / 1000.0);

            setSpeed (playSpeedMult);

            auto state = env->CallIntMethod (storedPlaybackState, AndroidPlaybackState.getState);

            if (state != PlaybackState::STATE_NONE && state != PlaybackState::STATE_STOPPED
                && state != PlaybackState::STATE_PAUSED && state != PlaybackState::STATE_ERROR)
            {
                play();
            }
        }

    private:
        struct PlaybackState
        {
            enum
            {
                STATE_NONE = 0,
                STATE_STOPPED = 1,
                STATE_PAUSED = 2,
                STATE_PLAYING = 3,
                STATE_FAST_FORWARDING = 4,
                STATE_REWINDING = 5,
                STATE_BUFFERING = 6,
                STATE_ERROR = 7,
                STATE_CONNECTING = 8,
                STATE_SKIPPING_TO_PREVIOUS = 9,
                STATE_SKIPPING_TO_NEXT = 10,
                STATE_SKIPPING_TO_QUEUE_ITEM = 11,
            };

            enum
            {
                ACTION_PAUSE              = 0x2,
                ACTION_PLAY               = 0x4,
                ACTION_PLAY_FROM_MEDIA_ID = 0x8000,
                ACTION_PLAY_PAUSE         = 0x200,
                ACTION_SEEK_TO            = 0x100,
                ACTION_STOP               = 0x1,
            };
        };

        //==============================================================================
        class Controller
        {
        public:
            Controller (MediaSession& ownerToUse, jobject nativeControllerToUse)
                : owner (ownerToUse),
                  nativeController (GlobalRef (nativeControllerToUse)),
                  controllerTransportControls (LocalRef<jobject> (getEnv()->CallObjectMethod (nativeController,
                                                                                              AndroidMediaController.getTransportControls))),
                  controllerCallback (LocalRef<jobject> (getEnv()->NewObject (AndroidMediaControllerCallback,
                                                                              AndroidMediaControllerCallback.constructor,
                                                                              android.activity.get(),
                                                                              reinterpret_cast<jlong> (this))))
            {
                auto* env = getEnv();

                env->CallVoidMethod (nativeController, AndroidMediaController.registerCallback, controllerCallback.get());
            }

            ~Controller()
            {
                auto* env = getEnv();
                env->CallVoidMethod (nativeController, AndroidMediaController.unregisterCallback, controllerCallback.get());
            }

            void load (const URL& url)
            {
                // NB: would use playFromUri, but it was only introduced in API 23...
                getEnv()->CallVoidMethod (controllerTransportControls, AndroidMediaControllerTransportControls.playFromMediaId,
                                          javaString (url.toString (true)).get(), nullptr);
            }

            void closeVideo()
            {
                getEnv()->CallVoidMethod (controllerTransportControls, AndroidMediaControllerTransportControls.stop);
            }

            void play()
            {
                getEnv()->CallVoidMethod (controllerTransportControls, AndroidMediaControllerTransportControls.play);
            }

            void stop()
            {
                // NB: calling pause, rather than stop, because after calling stop, we would have to call load() again.
                getEnv()->CallVoidMethod (controllerTransportControls, AndroidMediaControllerTransportControls.pause);
            }

            void setPosition (double newPosition)
            {
                auto seekPos = static_cast<jlong> (newPosition * 1000);

                getEnv()->CallVoidMethod (controllerTransportControls, AndroidMediaControllerTransportControls.seekTo, seekPos);
            }

            double getPosition() const
            {
                auto* env = getEnv();

                auto playbackState = LocalRef<jobject> (env->CallObjectMethod (nativeController, AndroidMediaController.getPlaybackState));

                if (playbackState != nullptr)
                    return env->CallLongMethod (playbackState, AndroidPlaybackState.getPosition) / 1000.0;

                return 0.0;
            }

            double getPlaySpeed() const
            {
                auto* env = getEnv();

                auto playbackState = LocalRef<jobject> (env->CallObjectMethod (nativeController, AndroidMediaController.getPlaybackState));

                if (playbackState != nullptr)
                    return (double) env->CallFloatMethod (playbackState, AndroidPlaybackState.getPlaybackSpeed);

                return 1.0;
            }

            void setVolume (float newVolume)
            {
                auto* env = getEnv();

                auto playbackInfo = LocalRef<jobject> (env->CallObjectMethod (nativeController, AndroidMediaController.getPlaybackInfo));

                auto maxVolume = env->CallIntMethod (playbackInfo, AndroidMediaControllerPlaybackInfo.getMaxVolume);

                auto targetVolume = jmin (jint (maxVolume * newVolume), maxVolume);

                static constexpr jint flagShowUI = 1;
                env->CallVoidMethod (nativeController, AndroidMediaController.setVolumeTo, targetVolume, flagShowUI);
            }

            float getVolume() const
            {
                auto* env = getEnv();

                auto playbackInfo = LocalRef<jobject> (env->CallObjectMethod (nativeController, AndroidMediaController.getPlaybackInfo));

                auto maxVolume = (int) (env->CallIntMethod (playbackInfo, AndroidMediaControllerPlaybackInfo.getMaxVolume));
                auto curVolume = (int) (env->CallIntMethod (playbackInfo, AndroidMediaControllerPlaybackInfo.getCurrentVolume));

                return static_cast<float> (curVolume) / maxVolume;
            }

        private:
            MediaSession& owner;

            GlobalRef nativeController;
            GlobalRef controllerTransportControls;
            GlobalRef controllerCallback;
            bool wasPlaying = false;
            bool wasPaused = true;

            //==============================================================================
            // MediaSessionController callbacks

            void audioInfoChanged (jobject info)
            {
                JUCE_VIDEO_LOG ("MediaSessionController::audioInfoChanged()");
                ignoreUnused (info);
            }

            void metadataChanged (jobject metadata)
            {
                JUCE_VIDEO_LOG ("MediaSessionController::metadataChanged()");
                ignoreUnused (metadata);
            }

            void playbackStateChanged (jobject playbackState)
            {
                JUCE_VIDEO_LOG ("MediaSessionController::playbackStateChanged()");

                if (playbackState == nullptr)
                    return;

                auto state = getEnv()->CallIntMethod (playbackState, AndroidPlaybackState.getState);

                static constexpr jint statePaused  = 2;
                static constexpr jint statePlaying = 3;

                if (wasPlaying == false && state == statePlaying)
                    owner.playbackStarted();
                else if (wasPaused == false && state == statePaused)
                    owner.playbackStopped();

                wasPlaying = state == statePlaying;
                wasPaused  = state == statePaused;
            }

            void sessionDestroyed()
            {
                JUCE_VIDEO_LOG ("MediaSessionController::sessionDestroyed()");
            }

            friend void juce_mediaControllerAudioInfoChanged (int64, void*);
            friend void juce_mediaControllerMetadataChanged (int64, void*);
            friend void juce_mediaControllerPlaybackStateChanged (int64, void*);
            friend void juce_mediaControllerSessionDestroyed (int64);
        };

        //==============================================================================
        class Player   : private MediaPlayerListener::Owner
        {
        public:
            Player (MediaSession& ownerToUse)
                : owner (ownerToUse),
                  mediaPlayerListener (*this),
                  nativeMediaPlayerListener (GlobalRef (CreateJavaInterface (&mediaPlayerListener,
                                                                             getNativeMediaPlayerListenerInterfaces())))

            {}

            void setDisplay (jobject surfaceHolder)
            {
                if (surfaceHolder == nullptr)
                {
                    videoSurfaceHolder.clear();

                    if (nativeMediaPlayer.get() != nullptr)
                        getEnv()->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setDisplay, nullptr);

                    return;
                }

                videoSurfaceHolder = GlobalRef (surfaceHolder);

                if (nativeMediaPlayer.get() != nullptr)
                    getEnv()->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setDisplay, videoSurfaceHolder.get());
            }

            void load (jstring mediaId, jobject extras)
            {
                ignoreUnused (extras);

                closeVideo();

                auto* env = getEnv();

                nativeMediaPlayer = GlobalRef (LocalRef<jobject> (env->NewObject (AndroidMediaPlayer, AndroidMediaPlayer.constructor)));

                currentState = State::idle;

                auto uri = LocalRef<jobject> (env->CallStaticObjectMethod (AndroidUri, AndroidUri.parse, mediaId));
                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setDataSource, android.activity.get(), uri.get());

                if (jniCheckHasExceptionOccurredAndClear())
                {
                    owner.errorOccurred ("Could not find video under path provided (" + juceString (mediaId) + ")");
                    return;
                }

                currentState = State::initialised;

                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setOnBufferingUpdateListener,   nativeMediaPlayerListener.get());
                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setOnCompletionListener, nativeMediaPlayerListener.get());
                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setOnErrorListener,      nativeMediaPlayerListener.get());
                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setOnInfoListener,       nativeMediaPlayerListener.get());
                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setOnPreparedListener,   nativeMediaPlayerListener.get());
                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setOnSeekCompleteListener,   nativeMediaPlayerListener.get());

                if (videoSurfaceHolder != nullptr)
                    env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setDisplay, videoSurfaceHolder.get());

                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.prepareAsync);

                currentState = State::preparing;
            }

            void closeVideo()
            {
                if (nativeMediaPlayer.get() == nullptr)
                    return;

                auto* env = getEnv();

                if (getCurrentStateInfo().canCallStop)
                    env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.stop);

                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.release);
                nativeMediaPlayer.clear();

                currentState = State::end;
            }

            bool isVideoOpen() const noexcept
            {
                return currentState == State::prepared || currentState == State::started
                    || currentState == State::paused || currentState == State::complete;
            }

            int getPlaybackStateFlag() const noexcept { return getCurrentStateInfo().playbackStateFlag; }
            int getAllowedActions()    const noexcept { return getCurrentStateInfo().allowedActions; }

            jlong getVideoDuration() const
            {
                if (! getCurrentStateInfo().canCallGetVideoDuration)
                    return 0;

                return getEnv()->CallIntMethod (nativeMediaPlayer, AndroidMediaPlayer.getDuration);
            }

            Rectangle<int> getVideoNativeSize() const
            {
                if (! getCurrentStateInfo().canCallGetVideoHeight)
                {
                    jassertfalse;
                    return {};
                }

                auto* env = getEnv();

                auto width  = (int) env->CallIntMethod (nativeMediaPlayer, AndroidMediaPlayer.getVideoWidth);
                auto height = (int) env->CallIntMethod (nativeMediaPlayer, AndroidMediaPlayer.getVideoHeight);

                return Rectangle<int> (0, 0, width, height);
            }

            void play()
            {
                if (! getCurrentStateInfo().canCallStart)
                {
                    jassertfalse;
                    return;
                }

                auto* env = getEnv();

                // Perform a potentially pending volume setting
                if (lastAudioVolume != std::numeric_limits<float>::min())
                    env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setVolume, (jfloat) lastAudioVolume, (jfloat) lastAudioVolume);

                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.start);

                currentState = State::started;
            }

            void pause()
            {
                if (! getCurrentStateInfo().canCallPause)
                {
                    jassertfalse;
                    return;
                }

                getEnv()->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.pause);

                currentState = State::paused;
            }

            bool isPlaying() const
            {
                return getCurrentStateInfo().isPlaying;
            }

            void setPlayPosition (jint newPositionMs)
            {
                if (! getCurrentStateInfo().canCallSeekTo)
                {
                    jassertfalse;
                    return;
                }

                getEnv()->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.seekTo, (jint) newPositionMs);
            }

            jint getPlayPosition() const
            {
                if (! getCurrentStateInfo().canCallGetCurrentPosition)
                    return 0.0;

                return getEnv()->CallIntMethod (nativeMediaPlayer, AndroidMediaPlayer.getCurrentPosition);
            }

            void setPlaySpeed (double newSpeed)
            {
                if (! getCurrentStateInfo().canCallSetPlaybackParams)
                {
                    jassertfalse;
                    return;
                }

                auto* env = getEnv();

                auto playbackParams = LocalRef<jobject> (env->CallObjectMethod (nativeMediaPlayer, AndroidMediaPlayer.getPlaybackParams));
                LocalRef<jobject> (env->CallObjectMethod (playbackParams, AndroidPlaybackParams.setSpeed, (jfloat) newSpeed));
                env->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setPlaybackParams, playbackParams.get());

                if (jniCheckHasExceptionOccurredAndClear())
                {
                    // MediaPlayer can't handle speed provided!
                    jassertfalse;
                }
            }

            double getPlaySpeed() const
            {
                if (! getCurrentStateInfo().canCallGetPlaybackParams)
                    return 0.0;

                auto* env = getEnv();

                auto playbackParams = LocalRef<jobject> (env->CallObjectMethod (nativeMediaPlayer, AndroidMediaPlayer.getPlaybackParams));
                return (double) env->CallFloatMethod (playbackParams, AndroidPlaybackParams.getSpeed);
            }

            void setAudioVolume (float newVolume)
            {
                if (! getCurrentStateInfo().canCallSetVolume)
                {
                    jassertfalse;
                    return;
                }

                lastAudioVolume = jlimit (0.0f, 1.0f, newVolume);

                if (nativeMediaPlayer.get() != nullptr)
                    getEnv()->CallVoidMethod (nativeMediaPlayer, AndroidMediaPlayer.setVolume, (jfloat) lastAudioVolume, (jfloat) lastAudioVolume);
            }

            float getAudioVolume() const
            {
                // There is NO getVolume() in MediaPlayer, so the value returned here can be incorrect!
                return lastAudioVolume;
            }

        private:
            //=============================================================================
            struct StateInfo
            {
                int playbackStateFlag = 0, allowedActions = 0;

                bool isPlaying, canCallGetCurrentPosition, canCallGetVideoDuration,
                        canCallGetVideoHeight, canCallGetVideoWidth, canCallGetPlaybackParams,
                        canCallPause, canCallPrepare, canCallSeekTo, canCallSetAudioAttributes,
                        canCallSetDataSource, canCallSetPlaybackParams, canCallSetVolume,
                        canCallStart, canCallStop;
            };

            enum class State
            {
                idle, initialised, preparing, prepared, started, paused, stopped, complete, error, end
            };

            static constexpr StateInfo stateInfos[] = {
                /* idle */
                {PlaybackState::STATE_NONE, PlaybackState::ACTION_PLAY_FROM_MEDIA_ID,
                            false, true, false, true, true, false, false, false, false, true,
                            true,  false, true, false, false},
                /* initialised */
                {PlaybackState::STATE_NONE, 0, // NB: could use action prepare, but that's API 24 onwards only
                            false, true, false, true, true, true, false, true, false, true,
                            false,  true, true, false, false},
                /* preparing */
                {PlaybackState::STATE_BUFFERING, 0,
                            false, false, false, false, false, true, false, false, false, false,
                            false,  false, false, false, false},
                /* prepared */
                {PlaybackState::STATE_PAUSED,
                            PlaybackState::ACTION_PLAY | PlaybackState::ACTION_PLAY_PAUSE | PlaybackState::ACTION_PLAY_FROM_MEDIA_ID | PlaybackState::ACTION_STOP | PlaybackState::ACTION_SEEK_TO,
                            false, true, true, true, true, true, false, false, true, true,
                            false, true, true, true, true},
                /* started */
                {PlaybackState::STATE_PLAYING,
                            PlaybackState::ACTION_PAUSE | PlaybackState::ACTION_PLAY_PAUSE | PlaybackState::ACTION_SEEK_TO | PlaybackState::ACTION_STOP | PlaybackState::ACTION_PLAY_FROM_MEDIA_ID,
                            true, true, true, true, true, true, true, false, true, true,
                            false, true, true, true, true},
                /* paused */
                {PlaybackState::STATE_PAUSED,
                            PlaybackState::ACTION_PLAY | PlaybackState::ACTION_PLAY_PAUSE | PlaybackState::ACTION_SEEK_TO | PlaybackState::ACTION_STOP | PlaybackState::ACTION_PLAY_FROM_MEDIA_ID,
                            false, true, true, true, true, true, true, false, true, true,
                            false, true, true, true, true},
                /* stopped */
                {PlaybackState::STATE_STOPPED,
                            PlaybackState::ACTION_PLAY_FROM_MEDIA_ID,
                            false, true, true, true, true, true, false, true, false, true,
                            false, false, true, false, true},
                /* complete */
                {PlaybackState::STATE_PAUSED,
                            PlaybackState::ACTION_SEEK_TO | PlaybackState::ACTION_STOP | PlaybackState::ACTION_PLAY_FROM_MEDIA_ID,
                            false, true, true, true, true, true, true, false, true, true,
                            false, true, true, true, true},
                /* error */
                {PlaybackState::STATE_ERROR,
                            PlaybackState::ACTION_PLAY_FROM_MEDIA_ID,
                            false, false, false, false, false, false, false, false, false, false,
                            false, false, false, false, false},
                /* end */
                {PlaybackState::STATE_NONE,
                            PlaybackState::ACTION_PLAY_FROM_MEDIA_ID,
                            false, false, false, false, false, false, false, false, false, false,
                            false, false, false, false, false}
            };

            StateInfo getCurrentStateInfo() const noexcept             { return stateInfos[static_cast<int> (currentState)]; }

            //==============================================================================
            MediaSession& owner;
            GlobalRef nativeMediaPlayer;

            MediaPlayerListener mediaPlayerListener;
            GlobalRef nativeMediaPlayerListener;

            float lastAudioVolume = std::numeric_limits<float>::min();

            GlobalRef videoSurfaceHolder;

            State currentState = State::idle;

            //==============================================================================
            void onPrepared (LocalRef<jobject>& mediaPlayer) override
            {
                JUCE_VIDEO_LOG ("MediaPlayer::onPrepared()");

                ignoreUnused (mediaPlayer);

                currentState = State::prepared;

                owner.playerPrepared();
            }

            void onBufferingUpdate (LocalRef<jobject>& mediaPlayer, int progress) override
            {
                ignoreUnused (mediaPlayer);

                owner.playerBufferingUpdated (progress);
            }

            void onSeekComplete (LocalRef<jobject>& mediaPlayer) override
            {
                JUCE_VIDEO_LOG ("MediaPlayer::onSeekComplete()");

                ignoreUnused (mediaPlayer);

                owner.playerSeekCompleted();
            }

            void onCompletion (LocalRef<jobject>& mediaPlayer) override
            {
                JUCE_VIDEO_LOG ("MediaPlayer::onCompletion()");

                ignoreUnused (mediaPlayer);

                currentState = State::complete;

                owner.playerPlaybackCompleted();
            }

            enum
            {
                MEDIA_INFO_UNKNOWN               = 1,
                MEDIA_INFO_VIDEO_RENDERING_START = 3,
                MEDIA_INFO_VIDEO_TRACK_LAGGING   = 700,
                MEDIA_INFO_BUFFERING_START       = 701,
                MEDIA_INFO_BUFFERING_END         = 702,
                MEDIA_INFO_NETWORK_BANDWIDTH     = 703,
                MEDIA_INFO_BAD_INTERLEAVING      = 800,
                MEDIA_INFO_NOT_SEEKABLE          = 801,
                MEDIA_INFO_METADATA_UPDATE       = 802,
                MEDIA_INFO_AUDIO_NOT_PLAYING     = 804,
                MEDIA_INFO_VIDEO_NOT_PLAYING     = 805,
                MEDIA_INFO_UNSUPPORTED_SUBTITE   = 901,
                MEDIA_INFO_SUBTITLE_TIMED_OUT    = 902
            };

            bool onInfo (LocalRef<jobject>& mediaPlayer, int what, int extra) override
            {
                JUCE_VIDEO_LOG ("MediaPlayer::onInfo(), infoCode: " + String (what) + " (" + infoCodeToString (what) + ")"
                                + ", extraCode: " + String (extra));

                ignoreUnused (mediaPlayer, extra);

                if (what == MEDIA_INFO_BUFFERING_START)
                    owner.playerBufferingStarted();
                else if (what == MEDIA_INFO_BUFFERING_END)
                    owner.playerBufferingEnded();

                return true;
            }

            static String infoCodeToString (int code)
            {
                switch (code)
                {
                    case MEDIA_INFO_UNKNOWN:               return "Unknown";
                    case MEDIA_INFO_VIDEO_RENDERING_START: return "Rendering start";
                    case MEDIA_INFO_VIDEO_TRACK_LAGGING:   return "Video track lagging";
                    case MEDIA_INFO_BUFFERING_START:       return "Buffering start";
                    case MEDIA_INFO_BUFFERING_END:         return "Buffering end";
                    case MEDIA_INFO_NETWORK_BANDWIDTH:     return "Network bandwidth info available";
                    case MEDIA_INFO_BAD_INTERLEAVING:      return "Bad interleaving";
                    case MEDIA_INFO_NOT_SEEKABLE:          return "Video not seekable";
                    case MEDIA_INFO_METADATA_UPDATE:       return "Metadata updated";
                    case MEDIA_INFO_AUDIO_NOT_PLAYING:     return "Audio not playing";
                    case MEDIA_INFO_VIDEO_NOT_PLAYING:     return "Video not playing";
                    case MEDIA_INFO_UNSUPPORTED_SUBTITE:   return "Unsupported subtitle";
                    case MEDIA_INFO_SUBTITLE_TIMED_OUT:    return "Subtitle timed out";
                    default:                               return "";
                }
            }

            bool onError (LocalRef<jobject>& mediaPlayer, int what, int extra) override
            {
                auto errorMessage = errorCodeToString (what);
                auto extraMessage = errorCodeToString (extra);

                if (extraMessage.isNotEmpty())
                    errorMessage << ", " << extraMessage;

                JUCE_VIDEO_LOG ("MediaPlayer::onError(), errorCode: " + String (what) + " (" + errorMessage + ")"
                                + ", extraCode: " + String (extra) + " (" + extraMessage + ")");

                ignoreUnused (mediaPlayer);

                currentState = State::error;

                owner.errorOccurred (errorMessage);
                return true;
            }

            static String errorCodeToString (int code)
            {
                enum
                {
                    MEDIA_ERROR_UNSUPPORTED                        = -1010,
                    MEDIA_ERROR_MALFORMED                          = -1007,
                    MEDIA_ERROR_IO                                 = -1004,
                    MEDIA_ERROR_TIMED_OUT                          = -110,
                    MEDIA_ERROR_UNKNOWN                            = 1,
                    MEDIA_ERROR_SERVER_DIED                        = 100,
                    MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK = 200
                };

                switch (code)
                {
                    case MEDIA_ERROR_UNSUPPORTED:                        return "Unsupported bitstream";
                    case MEDIA_ERROR_MALFORMED:                          return "Malformed bitstream";
                    case MEDIA_ERROR_IO:                                 return "File/Network I/O error";
                    case MEDIA_ERROR_TIMED_OUT:                          return "Timed out";
                    case MEDIA_ERROR_UNKNOWN:                            return "Unknown error";
                    case MEDIA_ERROR_SERVER_DIED:                        return "Media server died (playback restart required)";
                    case MEDIA_ERROR_NOT_VALID_FOR_PROGRESSIVE_PLAYBACK: return "Video container not valid for progressive playback";
                    default:                                             return "";
                }
            }

            //==============================================================================
            static StringArray getNativeMediaPlayerListenerInterfaces()
            {
                #define IFPREFIX "android/media/MediaPlayer$"

                return { IFPREFIX "OnCompletionListener", IFPREFIX "OnErrorListener",
                         IFPREFIX "OnInfoListener", IFPREFIX "OnPreparedListener",
                         IFPREFIX "OnBufferingUpdateListener", IFPREFIX "OnSeekCompleteListener"
                };

                #undef IFPREFIX
            }
        };

        //==============================================================================
        Pimpl& owner;

        int sdkVersion;

        GlobalRef audioAttributes;
        GlobalRef nativeMediaSession;
        GlobalRef mediaSessionCallback;
        GlobalRef playbackStateBuilder;

        Controller controller;
        Player player;

        GlobalRef audioManager;
        AudioManagerOnAudioFocusChangeListener audioFocusChangeListener;
        GlobalRef nativeAudioFocusChangeListener;
        GlobalRef audioFocusRequest;

        GlobalRef storedPlaybackState;

        bool pendingSeekRequest = false;

        bool playerBufferingInProgress = false;
        bool usesBuffering = false;
        SparseSet<int> bufferedRegions;

        double playSpeedMult = 1.0;
        bool hasAudioFocus = false;

        //==============================================================================
        // MediaSession callbacks

        void pauseCallback()
        {
            JUCE_VIDEO_LOG ("MediaSession::pauseCallback()");

            player.pause();
            updatePlaybackState();

            abandonAudioFocus();
        }

        void playCallback()
        {
            JUCE_VIDEO_LOG ("MediaSession::playCallback()");

            requestAudioFocus();

            if (! hasAudioFocus)
            {
                errorOccurred ("Application has been denied audio focus. Try again later.");
                return;
            }

            getEnv()->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setActive, true);

            player.play();
            setSpeed (playSpeedMult);
            updatePlaybackState();
        }

        void playFromMediaIdCallback (jstring mediaId, jobject extras)
        {
            JUCE_VIDEO_LOG ("MediaSession::playFromMediaIdCallback()");

            player.load (mediaId, extras);
            updatePlaybackState();
        }

        void seekToCallback (jlong pos)
        {
            JUCE_VIDEO_LOG ("MediaSession::seekToCallback()");

            pendingSeekRequest = true;
            player.setPlayPosition ((jint) pos);
            updatePlaybackState();
        }

        void stopCallback()
        {
            JUCE_VIDEO_LOG ("MediaSession::stopCallback()");

            auto* env = getEnv();

            env->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setActive, false);

            player.closeVideo();
            updatePlaybackState();

            abandonAudioFocus();

            owner.closeVideoFinished();
        }

        //==============================================================================
        bool isSeekInProgress() const noexcept
        {
            if (pendingSeekRequest)
                return true;

            if (! usesBuffering)
                return false;

            // NB: player sometimes notifies us about buffering, but only for regions that
            // were previously buffered already. For buffering happening for the first time,
            // we don't get such notification...
            if (playerBufferingInProgress)
                return true;

            auto playPos = player.getPlayPosition();
            auto durationMs = player.getVideoDuration();
            auto playPosPercent = (int) (100 * playPos / static_cast<double> (durationMs));

            // NB: assuming the playback will start roughly when there is 5% of content loaded...
            return ! bufferedRegions.containsRange (Range<int> (playPosPercent, jmin (101, playPosPercent + 5)));
        }

        void updatePlaybackState()
        {
            getEnv()->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setPlaybackState, getCurrentPlaybackState());
        }

        jobject getCurrentPlaybackState()
        {
            static constexpr int bufferingState = 6;

            auto playbackStateFlag = isSeekInProgress() ? bufferingState : player.getPlaybackStateFlag();
            auto playPos = player.getPlayPosition();
            auto playSpeed = player.getPlaySpeed();
            auto allowedActions = player.getAllowedActions();

            auto* env = getEnv();

            LocalRef<jobject> (env->CallObjectMethod (playbackStateBuilder, AndroidPlaybackStateBuilder.setState,
                                                      (jint) playbackStateFlag, (jlong) playPos, (jfloat) playSpeed));

            LocalRef<jobject> (env->CallObjectMethod (playbackStateBuilder, AndroidPlaybackStateBuilder.setActions, (jint) allowedActions));

            return env->CallObjectMethod (playbackStateBuilder, AndroidPlaybackStateBuilder.build);
        }

        //==============================================================================
        void playerPrepared()
        {
            resetState();

            updateMetadata();

            owner.loadFinished();
        }

        void playerBufferingStarted() { playerBufferingInProgress = true; }
        void playerBufferingEnded()   { playerBufferingInProgress = false; }

        void playerBufferingUpdated (int progress)
        {
            usesBuffering = true;

            updatePlaybackState();

            auto playPos = player.getPlayPosition();
            auto durationMs = player.getVideoDuration();
            auto playPosPercent = (int) (100 * playPos / static_cast<double> (durationMs));

            bufferedRegions.addRange (Range<int> (playPosPercent, progress + 1));

            String ranges;

            for (auto& r : bufferedRegions.getRanges())
                ranges << "[" << r.getStart() << "%, " << r.getEnd() - 1 << "%] ";

            JUCE_VIDEO_LOG ("Buffering status update, seek pos: " + String (playPosPercent) + "%, buffered regions: " + ranges);
        }

        void playerSeekCompleted()
        {
            pendingSeekRequest = false;
            updatePlaybackState();
        }

        void playerPlaybackCompleted()
        {
            pauseCallback();
            seekToCallback ((jlong) 0);
        }

        void updateMetadata()
        {
            auto* env = getEnv();

            auto metadataBuilder = LocalRef<jobject> (env->NewObject (AndroidMediaMetadataBuilder,
                                                                      AndroidMediaMetadataBuilder.constructor));

            auto durationMs = player.getVideoDuration();

            auto jDurationKey = javaString ("android.media.metadata.DURATION");
            LocalRef<jobject> (env->CallObjectMethod (metadataBuilder,
                                                      AndroidMediaMetadataBuilder.putLong,
                                                      jDurationKey.get(),
                                                      (jlong) durationMs));

            auto jNumTracksKey = javaString ("android.media.metadata.NUM_TRACKS");
            LocalRef<jobject> (env->CallObjectMethod (metadataBuilder,
                                                      AndroidMediaMetadataBuilder.putLong,
                                                      jNumTracksKey.get(),
                                                      (jlong) 1));

            env->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setMetadata,
                                 env->CallObjectMethod (metadataBuilder, AndroidMediaMetadataBuilder.build));
        }

        void errorOccurred (const String& errorMessage)
        {
            auto* env = getEnv();

            // Propagate error to session controller(s) and ...
            LocalRef<jobject> (env->CallObjectMethod (playbackStateBuilder, AndroidPlaybackStateBuilder.setErrorMessage,
                                                      javaString (errorMessage).get()));

            auto state = LocalRef<jobject> (env->CallObjectMethod (playbackStateBuilder, AndroidPlaybackStateBuilder.build));
            env->CallVoidMethod (nativeMediaSession, AndroidMediaSession.setPlaybackState, state.get());

            // ...also notify JUCE side client
            owner.errorOccurred (errorMessage);
        }

        //==============================================================================
        static jobject createAudioFocusRequestIfNecessary (int sdkVersion, const GlobalRef& audioAttributes,
                                                           const GlobalRef& nativeAudioFocusChangeListener)
        {
            if (sdkVersion < 26)
                return nullptr;

            auto* env = getEnv();

            auto requestBuilderClass = LocalRef<jclass> (env->FindClass ("android/media/AudioFocusRequest$Builder"));

            static jmethodID constructor = env->GetMethodID (requestBuilderClass, "<init>", "(I)V");
            static jmethodID buildMethod = env->GetMethodID (requestBuilderClass, "build", "()Landroid/media/AudioFocusRequest;");
            static jmethodID setAudioAttributesMethod = env->GetMethodID (requestBuilderClass, "setAudioAttributes",
                                                                          "(Landroid/media/AudioAttributes;)Landroid/media/AudioFocusRequest$Builder;");
            static jmethodID setOnAudioFocusChangeListenerMethod = env->GetMethodID (requestBuilderClass, "setOnAudioFocusChangeListener",
                                                                                     "(Landroid/media/AudioManager$OnAudioFocusChangeListener;)Landroid/media/AudioFocusRequest$Builder;");

            static constexpr jint audioFocusGain = 1;

            auto requestBuilder = LocalRef<jobject> (env->NewObject (requestBuilderClass, constructor, audioFocusGain));
            LocalRef<jobject> (env->CallObjectMethod (requestBuilder, setAudioAttributesMethod, audioAttributes.get()));
            LocalRef<jobject> (env->CallObjectMethod (requestBuilder, setOnAudioFocusChangeListenerMethod, nativeAudioFocusChangeListener.get()));

            return env->CallObjectMethod (requestBuilder, buildMethod);
        }

        void requestAudioFocus()
        {
            static constexpr jint audioFocusGain = 1;
            static constexpr jint streamMusic = 3;
            static constexpr jint audioFocusRequestGranted = 1;

            jint result = audioFocusRequestGranted;

            if (sdkVersion >= 26)
            {
                static jmethodID requestAudioFocusMethod = getEnv()->GetMethodID (AndroidAudioManager, "requestAudioFocus",
                                                                                  "(Landroid/media/AudioFocusRequest;)I");

                result = getEnv()->CallIntMethod (audioManager, requestAudioFocusMethod, audioFocusRequest.get());
            }
            else
            {
                result = getEnv()->CallIntMethod (audioManager, AndroidAudioManager.requestAudioFocus,
                                                  nativeAudioFocusChangeListener.get(), streamMusic, audioFocusGain);
            }

            hasAudioFocus = result == audioFocusRequestGranted;
        }

        void abandonAudioFocus()
        {
            if (! hasAudioFocus)
                return;

            static constexpr jint audioFocusRequestGranted = 1;

            jint result = audioFocusRequestGranted;

            if (sdkVersion >= 26)
            {
                static jmethodID abandonAudioFocusMethod = getEnv()->GetMethodID (AndroidAudioManager, "abandonAudioFocusRequest",
                                                                                  "(Landroid/media/AudioFocusRequest;)I");

                result = getEnv()->CallIntMethod (audioManager, abandonAudioFocusMethod, audioFocusRequest.get());
            }
            else
            {
                result = getEnv()->CallIntMethod (audioManager, AndroidAudioManager.abandonAudioFocus,
                                                   nativeAudioFocusChangeListener.get());
            }

            // NB: granted in this case means "granted to change the focus to abandoned"...
            hasAudioFocus = result != audioFocusRequestGranted;
        }

        void onAudioFocusChange (int changeType) override
        {
            static constexpr jint audioFocusGain = 1;

            if (changeType == audioFocusGain)
                JUCE_VIDEO_LOG ("Audio focus gained");
            else
                JUCE_VIDEO_LOG ("Audio focus lost");

            if (changeType != audioFocusGain)
            {
                if (isPlaying())
                {
                    JUCE_VIDEO_LOG ("Received a request to abandon audio focus. Stopping playback...");
                    stop();
                }

                abandonAudioFocus();
            }
        }

        //==============================================================================
        void playbackStarted()
        {
            owner.playbackStarted();
        }

        void playbackStopped()
        {
            owner.playbackStopped();
        }

        //==============================================================================
        void resetState()
        {
            usesBuffering = false;
            bufferedRegions.clear();
            playerBufferingInProgress = false;

            pendingSeekRequest = false;

            playSpeedMult = 1.0;
            hasAudioFocus = false;
        }

        //==============================================================================
        static jobject getAudioAttributes()
        {
            auto* env = getEnv();

            auto audioAttribsBuilder = LocalRef<jobject> (env->NewObject (AndroidAudioAttributesBuilder,
                                                                          AndroidAudioAttributesBuilder.constructor));
            static constexpr jint contentTypeMovie = 3;
            static constexpr jint usageMedia = 1;

            LocalRef<jobject> (env->CallObjectMethod (audioAttribsBuilder, AndroidAudioAttributesBuilder.setContentType, contentTypeMovie));
            LocalRef<jobject> (env->CallObjectMethod (audioAttribsBuilder, AndroidAudioAttributesBuilder.setUsage, usageMedia));

            return env->CallObjectMethod (audioAttribsBuilder, AndroidAudioAttributesBuilder.build);
        }

        friend void juce_mediaSessionPause (int64);
        friend void juce_mediaSessionPlay (int64);
        friend void juce_mediaSessionPlayFromMediaId (int64, void*, void*);
        friend void juce_mediaSessionSeekTo (int64, int64);
        friend void juce_mediaSessionStop (int64);

        friend void juce_mediaControllerAudioInfoChanged (int64, void*);
        friend void juce_mediaControllerMetadataChanged (int64, void*);
        friend void juce_mediaControllerPlaybackStateChanged (int64, void*);
        friend void juce_mediaControllerSessionDestroyed (int64);
    };

   #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
    //==============================================================================
    class SystemVolumeListener
    {
    public:
        SystemVolumeListener (Pimpl& ownerToUse)
            : owner (ownerToUse),
              nativeObserver (LocalRef<jobject> (getEnv()->NewObject (SystemVolumeObserver,
                                                                      SystemVolumeObserver.constructor,
                                                                      android.activity.get(),
                                                                      android.activity.get(),
                                                                      reinterpret_cast<jlong> (this))))
        {
            setEnabled (true);
        }

        ~SystemVolumeListener()
        {
            setEnabled (false);
        }

        void setEnabled (bool shouldBeEnabled)
        {
            getEnv()->CallVoidMethod (nativeObserver, SystemVolumeObserver.setEnabled, shouldBeEnabled);

            // Send first notification instantly to ensure sync.
            if (shouldBeEnabled)
                systemVolumeChanged();
        }

    private:
        Pimpl& owner;
        GlobalRef nativeObserver;

        void systemVolumeChanged()
        {
            WeakReference<SystemVolumeListener> weakThis (this);

            MessageManager::callAsync ([weakThis]() mutable
                                       {
                                           if (weakThis == nullptr)
                                               return;

                                           if (weakThis->owner.owner.onGlobalMediaVolumeChanged != nullptr)
                                               weakThis->owner.owner.onGlobalMediaVolumeChanged();
                                       });
        }

        friend void juce_mediaSessionSystemVolumeChanged (int64);

        JUCE_DECLARE_WEAK_REFERENCEABLE (SystemVolumeListener)
    };
   #endif

    //==============================================================================
    VideoComponent& owner;

    MediaSession mediaSession;
    AppPausedResumedListener appPausedResumedListener;
    GlobalRef appPausedResumedListenerNative;
   #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
    SystemVolumeListener systemVolumeListener;
   #endif

    std::function<void (const URL&, Result)> loadFinishedCallback;

    bool wasOpen = false;

    //==============================================================================
    void loadFinished()
    {
        owner.resized();

        if (loadFinishedCallback != nullptr)
        {
            loadFinishedCallback (currentURL, Result::ok());
            loadFinishedCallback = nullptr;
        }
    }

    void closeVideoFinished()
    {
        owner.resized();
    }

    void errorOccurred (const String& errorMessage)
    {
        if (owner.onErrorOccurred != nullptr)
            owner.onErrorOccurred (errorMessage);
    }

    void playbackStarted()
    {
        if (owner.onPlaybackStarted != nullptr)
            owner.onPlaybackStarted();
    }

    void playbackStopped()
    {
        if (owner.onPlaybackStopped != nullptr)
            owner.onPlaybackStopped();
    }

    void videoSurfaceChanged (jobject surfaceHolder)
    {
        mediaSession.setDisplay (surfaceHolder);
    }

    void videoSurfaceDestroyed (jobject /*surfaceHolder*/)
    {
        mediaSession.setDisplay (nullptr);
    }

    //==============================================================================
    void appPaused() override
    {
        wasOpen = isOpen();

        if (! wasOpen)
            return;

        JUCE_VIDEO_LOG ("App paused, releasing media player...");

        mediaSession.storeState();
        mediaSession.closeVideo();

       #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
        systemVolumeListener.setEnabled (false);
       #endif
    }

    void appResumed() override
    {
        if (! wasOpen)
            return;

        JUCE_VIDEO_LOG ("App resumed, restoring media player...");

        loadAsync (currentURL, [this](const URL&, Result r)
                   {
                       if (r.wasOk())
                           mediaSession.restoreState();
                   });

       #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
        systemVolumeListener.setEnabled (true);
       #endif
    }

    //==============================================================================
    friend void juce_surfaceChangedNativeVideo (int64, void*);
    friend void juce_surfaceDestroyedNativeVideo (int64, void*);

    friend void juce_mediaSessionPause (int64);
    friend void juce_mediaSessionPlay (int64);
    friend void juce_mediaSessionPlayFromMediaId (int64, void*, void*);
    friend void juce_mediaSessionSeekTo (int64, int64);
    friend void juce_mediaSessionStop (int64);

    friend void juce_mediaControllerAudioInfoChanged (int64, void*);
    friend void juce_mediaControllerMetadataChanged (int64, void*);
    friend void juce_mediaControllerPlaybackStateChanged (int64, void*);
    friend void juce_mediaControllerSessionDestroyed (int64);

    friend void juce_mediaSessionSystemVolumeChanged (int64);
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

#if __ANDROID_API__ >= 21
//==============================================================================
void juce_surfaceChangedNativeVideo (int64 host, void* surfaceHolder)
{
    reinterpret_cast<VideoComponent::Pimpl*> (host)->videoSurfaceChanged (static_cast<jobject> (surfaceHolder));
}

void juce_surfaceDestroyedNativeVideo (int64 host, void* surfaceHolder)
{
    reinterpret_cast<VideoComponent::Pimpl*> (host)->videoSurfaceDestroyed (static_cast<jobject> (surfaceHolder));
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024NativeSurfaceView), dispatchDrawNativeVideo,
                   void, (JNIEnv* env, jobject nativeView, jlong host, jobject canvas))
{
    ignoreUnused (nativeView, host, canvas);
    setEnv (env);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024NativeSurfaceView), surfaceChangedNativeVideo,
                   void, (JNIEnv* env, jobject nativeView, jlong host, jobject holder, jint format, jint width, jint height))
{
    ignoreUnused (nativeView, format, width, height);
    setEnv (env);

    JUCE_VIDEO_LOG ("video surface changed");

    juce_surfaceChangedNativeVideo (host, holder);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024NativeSurfaceView), surfaceCreatedNativeVideo,
                   void, (JNIEnv* env, jobject nativeView, jlong host, jobject holder))
{
    ignoreUnused (nativeView, host, holder);
    setEnv (env);

    JUCE_VIDEO_LOG ("video surface created");
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024NativeSurfaceView), surfaceDestroyedNativeVideo,
                   void, (JNIEnv* env, jobject nativeView, jlong host, jobject holder))
{
    ignoreUnused (nativeView, host, holder);
    setEnv (env);

    JUCE_VIDEO_LOG ("video surface destroyed");
    juce_surfaceDestroyedNativeVideo (host, holder);
}

//==============================================================================
void juce_mediaSessionPause (int64 host)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession*> (host)->pauseCallback();
}

void juce_mediaSessionPlay (int64 host)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession*> (host)->playCallback();
}

void juce_mediaSessionPlayFromMediaId (int64 host, void* mediaId, void* extras)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession*> (host)->playFromMediaIdCallback ((jstring) mediaId, (jobject) extras);
}

void juce_mediaSessionSeekTo (int64 host, int64 pos)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession*> (host)->seekToCallback (pos);
}

void juce_mediaSessionStop (int64 host)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession*> (host)->stopCallback();
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaSessionCallback), mediaSessionPause,
                   void, (JNIEnv* env, jobject /*mediaSessionCallback*/, jlong host))
{
    setEnv (env);
    juce_mediaSessionPause (host);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaSessionCallback), mediaSessionPlay,
                   void, (JNIEnv* env, jobject /*mediaSessionCallback*/, jlong host))
{
    setEnv (env);
    juce_mediaSessionPlay (host);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaSessionCallback), mediaSessionPlayFromMediaId,
                   void, (JNIEnv* env, jobject /*mediaSessionCallback*/, jlong host, jobject mediaId, jobject extras))
{
    setEnv (env);
    juce_mediaSessionPlayFromMediaId (host, mediaId, extras);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaSessionCallback), mediaSessionSeekTo,
                   void, (JNIEnv* env, jobject /*mediaSessionCallback*/, jlong host, jlong pos))
{
    setEnv (env);
    juce_mediaSessionSeekTo (host, pos);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaSessionCallback), mediaSessionStop,
                   void, (JNIEnv* env, jobject /*mediaSessionCallback*/, jlong host))
{
    setEnv (env);
    juce_mediaSessionStop (host);
}

//==============================================================================
void juce_mediaControllerAudioInfoChanged (int64 host, void* info)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession::Controller*> (host)->audioInfoChanged ((jobject) info);
}

void juce_mediaControllerMetadataChanged (int64 host, void* metadata)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession::Controller*> (host)->metadataChanged ((jobject) metadata);
}

void juce_mediaControllerPlaybackStateChanged (int64 host, void* state)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession::Controller*> (host)->playbackStateChanged ((jobject) state);
}

void juce_mediaControllerSessionDestroyed (int64 host)
{
    reinterpret_cast<VideoComponent::Pimpl::MediaSession::Controller*> (host)->sessionDestroyed();
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaControllerCallback), mediaControllerAudioInfoChanged,
                   void, (JNIEnv* env, jobject /*mediaControllerCallback*/, jlong host, jobject playbackInfo))
{
    setEnv (env);
    juce_mediaControllerAudioInfoChanged (host, playbackInfo);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaControllerCallback), mediaControllerMetadataChanged,
                   void, (JNIEnv* env, jobject /*mediaControllerCallback*/, jlong host, jobject metadata))
{
    setEnv (env);
    juce_mediaControllerMetadataChanged (host, metadata);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaControllerCallback), mediaControllerPlaybackStateChanged,
                   void, (JNIEnv* env, jobject /*mediaControllerCallback*/, jlong host, jobject playbackState))
{
    setEnv (env);
    juce_mediaControllerPlaybackStateChanged (host, playbackState);
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024MediaControllerCallback), mediaControllerSessionDestroyed,
                   void, (JNIEnv* env, jobject /*mediaControllerCallback*/, jlong host))
{
    setEnv (env);
    juce_mediaControllerSessionDestroyed (host);
}

//==============================================================================
void juce_mediaSessionSystemVolumeChanged (int64 host)
{
   #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
    reinterpret_cast<VideoComponent::Pimpl::SystemVolumeListener*> (host)->systemVolumeChanged();
   #else
    ignoreUnused (host);
   #endif
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024SystemVolumeObserver), mediaSessionSystemVolumeChanged,
                   void, (JNIEnv* env, jobject /*systemSettingsObserver*/, jlong host))
{
    setEnv (env);
    juce_mediaSessionSystemVolumeChanged (host);
}

//==============================================================================
constexpr VideoComponent::Pimpl::MediaSession::Player::StateInfo VideoComponent::Pimpl::MediaSession::Player::stateInfos[];
#endif
