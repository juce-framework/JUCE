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

extern jobject createOpenGLView (ComponentPeer*);

//==============================================================================
class OpenGLContext::NativeContext
{
public:
    NativeContext (Component& component_,
                   const OpenGLPixelFormat& pixelFormat,
                   const NativeContext* contextToShareWith)
        : component (component_),
          isInsideGLCallback (false)
    {
        glView = GlobalRef (createOpenGLView (component.getPeer()));

        {
            const ScopedLock sl (getContextListLock());
            getContextList().add (this);
        }

        updateWindowPosition (component.getTopLevelComponent()
                                ->getLocalArea (&component, component.getLocalBounds()));
    }

    ~NativeContext()
    {
        {
            const ScopedLock sl (getContextListLock());
            getContextList().removeValue (this);
        }

        android.activity.callVoidMethod (JuceAppActivity.deleteView, glView.get());
        glView.clear();
    }

    void initialiseOnRenderThread()
    {
    }

    void shutdownOnRenderThread()
    {
    }

    bool makeActive() const noexcept            { return isInsideGLCallback; }
    bool makeInactive() const noexcept          { return true; }
    bool isActive() const noexcept              { return isInsideGLCallback; }

    void swapBuffers() const noexcept           {}
    bool setSwapInterval (const int)            { return false; }
    int getSwapInterval() const                 { return 0; }

    bool createdOk() const noexcept             { return getRawContext() != nullptr; }
    void* getRawContext() const noexcept        { return glView.get(); }
    GLuint getFrameBufferID() const noexcept    { return 0; }

    void updateWindowPosition (const Rectangle<int>& bounds)
    {
        if (lastBounds != bounds)
        {
            lastBounds = bounds;
            glView.callVoidMethod (OpenGLView.layout,
                                   bounds.getX(), bounds.getY(),
                                   bounds.getRight(), bounds.getBottom());
        }
    }

    void triggerRepaint()
    {
        glView.callVoidMethod (OpenGLView.requestRender);
    }

    //==============================================================================
    void contextCreatedCallback();
    void renderCallback();

    //==============================================================================
    static NativeContext* findContextFor (JNIEnv* env, jobject glView)
    {
        const ScopedLock sl (getContextListLock());
        const ContextArray& contexts = getContextList();

        for (int i = contexts.size(); --i >= 0;)
        {
            NativeContext* const c = contexts.getUnchecked(i);

            if (env->IsSameObject (c->glView.get(), glView))
                return c;
        }

        return nullptr;
    }

    static NativeContext* getActiveContext() noexcept
    {
        const ScopedLock sl (getContextListLock());
        const ContextArray& contexts = getContextList();

        for (int i = contexts.size(); --i >= 0;)
        {
            NativeContext* const c = contexts.getUnchecked(i);

            if (c->isInsideGLCallback)
                return c;
        }

        return nullptr;
    }

    Component& component;

private:
    GlobalRef glView;
    Rectangle<int> lastBounds;
    bool isInsideGLCallback;

    static CriticalSection& getContextListLock()
    {
        static CriticalSection lock;
        return lock;
    }

    typedef Array<NativeContext*> ContextArray;

    static ContextArray& getContextList()
    {
        static ContextArray contexts;
        return contexts;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext);
};

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return OpenGLContext::NativeContext::getActiveContext() != nullptr;
}

//==============================================================================
JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024OpenGLView), contextCreated, void, (JNIEnv* env, jobject view))
{
    threadLocalJNIEnvHolder.getOrAttach();

    JUCE_CHECK_OPENGL_ERROR

    for (int i = 100; --i >= 0;)
    {
        OpenGLContext::NativeContext* const context = OpenGLContext::NativeContext::findContextFor (env, view);

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

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024OpenGLView), contextChangedSize, void, (JNIEnv* env, jobject view))
{
}

JUCE_JNI_CALLBACK (JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024OpenGLView), render, void, (JNIEnv* env, jobject view))
{
    OpenGLContext::NativeContext* const context = OpenGLContext::NativeContext::findContextFor (env, view);

    if (context != nullptr)
        context->renderCallback();
}
