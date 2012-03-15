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


#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD) \
 METHOD (layout,        "layout",        "(IIII)V") \
 METHOD (requestRender, "requestRender", "()V") \

DECLARE_JNI_CLASS (OpenGLView, JUCE_ANDROID_ACTIVITY_CLASSPATH "$OpenGLView");
#undef JNI_CLASS_MEMBERS

jobject createOpenGLView (ComponentPeer*);

//==============================================================================
class AndroidGLContext   : public OpenGLContext
{
public:
    AndroidGLContext (OpenGLComponent* const component_,
                      ComponentPeer* peer,
                      const OpenGLPixelFormat& pixelFormat,
                      const AndroidGLContext* const sharedContext,
                      const bool isGLES2_)
        : component (component_),
          lastWidth (0), lastHeight (0),
          isGLES2 (isGLES2_),
          isInsideGLCallback (false)
    {
        jassert (peer != nullptr);

        glView = GlobalRef (createOpenGLView (peer));

        const ScopedLock sl (getContextListLock());
        getContextList().add (this);
    }

    ~AndroidGLContext()
    {
        properties.clear(); // to release any stored programs, etc that may be held in properties.

        android.activity.callVoidMethod (JuceAppActivity.deleteView, glView.get());
        glView.clear();

        const ScopedLock sl (getContextListLock());
        getContextList().removeValue (this);
    }

    //==============================================================================
    bool makeActive() const noexcept                { return isInsideGLCallback; }
    bool makeInactive() const noexcept              { return true; }
    bool isActive() const noexcept                  { return isInsideGLCallback; }

    void swapBuffers() {}

    void* getRawContext() const noexcept            { return glView.get(); }
    unsigned int getFrameBufferID() const           { return 0; }

    int getWidth() const                            { return lastWidth; }
    int getHeight() const                           { return lastHeight; }

    bool areShadersAvailable() const                { return isGLES2; }

    void updateWindowPosition (const Rectangle<int>& bounds)
    {
        if (lastWidth != bounds.getWidth() || lastHeight != bounds.getHeight())
        {
            lastWidth  = bounds.getWidth();
            lastHeight = bounds.getHeight();

            glView.callVoidMethod (OpenGLView.layout,
                                   bounds.getX(), bounds.getY(),
                                   bounds.getRight(), bounds.getBottom());
        }
    }

    bool setSwapInterval (const int /*numFramesPerSwap*/)   { return false; }
    int getSwapInterval() const                             { return 0; }

    //==============================================================================
    void contextCreatedCallback()
    {
        properties.clear();

        isInsideGLCallback = true;

        if (component != nullptr)
            dynamic_cast <OpenGLComponent*> (component.get())->newOpenGLContextCreated();

        isInsideGLCallback = false;
    }

    void renderCallback()
    {
        isInsideGLCallback = true;

        if (component != nullptr)
            dynamic_cast <OpenGLComponent*> (component.get())->performRender();

        isInsideGLCallback = false;
    }

    //==============================================================================
    static CriticalSection& getContextListLock()
    {
        static CriticalSection lock;
        return lock;
    }

    static Array<AndroidGLContext*>& getContextList()
    {
        static Array <AndroidGLContext*> contexts;
        return contexts;
    }

    static AndroidGLContext* findContextFor (JNIEnv* env, jobject glView)
    {
        const ScopedLock sl (getContextListLock());
        const Array<AndroidGLContext*>& contexts = getContextList();

        for (int i = contexts.size(); --i >= 0;)
        {
            AndroidGLContext* const c = contexts.getUnchecked(i);

            if (env->IsSameObject (c->glView.get(), glView))
                return c;
        }

        return nullptr;
    }

    static bool isAnyContextActive()
    {
        const ScopedLock sl (getContextListLock());
        const Array<AndroidGLContext*>& contexts = getContextList();

        for (int i = contexts.size(); --i >= 0;)
        {
            const AndroidGLContext* const c = contexts.getUnchecked(i);

            if (c->isInsideGLCallback)
                return true;
        }

        return false;
    }

    //==============================================================================
    GlobalRef glView;

private:
    WeakReference<Component> component;
    int lastWidth, lastHeight;
    bool isGLES2;
    bool isInsideGLCallback;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidGLContext);
};

//==============================================================================
OpenGLContext* OpenGLComponent::createContext()
{
    ComponentPeer* peer = getTopLevelComponent()->getPeer();

    if (peer != nullptr)
        return new AndroidGLContext (this, peer, preferredPixelFormat,
                                     dynamic_cast <const AndroidGLContext*> (contextToShareListsWith),
                                     (flags & openGLES1) == 0);

    return nullptr;
}

bool OpenGLHelpers::isContextActive()
{
    return AndroidGLContext::isAnyContextActive();
}

void triggerAndroidOpenGLRepaint (OpenGLContext* context)
{
    AndroidGLContext* c = dynamic_cast <AndroidGLContext*> (context);

    if (c != nullptr)
        c->glView.callVoidMethod (OpenGLView.requestRender);
}

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024OpenGLView), contextCreated, void, (JNIEnv* env, jobject view))
{
    JUCE_CHECK_OPENGL_ERROR

    for (int i = 100; --i >= 0;)
    {
        AndroidGLContext* const context = AndroidGLContext::findContextFor (env, view);

        if (context != nullptr)
        {
            context->contextCreatedCallback();
            JUCE_CHECK_OPENGL_ERROR
            return;
        }

        Thread::sleep (20);
    }

    jassertfalse;
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024OpenGLView), render, void, (JNIEnv* env, jobject view))
{
    AndroidGLContext* const context = AndroidGLContext::findContextFor (env, view);

    if (context != nullptr)
        context->renderCallback();
}
