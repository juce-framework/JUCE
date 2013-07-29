/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
    NativeContext (Component& comp,
                   const OpenGLPixelFormat& pixelFormat,
                   void* /*contextToShareWith*/,
                   bool /*useMultisampling*/)
        : component (comp),
          isInsideGLCallback (false)
    {
        {
            const ScopedLock sl (contextListLock);
            glView = GlobalRef (createOpenGLView (component.getPeer()));
            contextList.add (this);
        }

        updateWindowPosition (component.getTopLevelComponent()
                                ->getLocalArea (&component, component.getLocalBounds()));
    }

    ~NativeContext()
    {
        {
            const ScopedLock sl (contextListLock);
            contextList.removeFirstMatchingValue (this);
        }

        android.activity.callVoidMethod (JuceAppActivity.deleteView, glView.get());
        glView.clear();
    }

    void initialiseOnRenderThread (OpenGLContext&) {}
    void shutdownOnRenderThread() {}

    bool makeActive() const noexcept            { return isInsideGLCallback; }
    bool isActive() const noexcept              { return isInsideGLCallback; }
    static void deactivateCurrentContext()      {}

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
    void contextChangedSize() {}
    void renderCallback();

    //==============================================================================
    static NativeContext* findContextFor (JNIEnv* env, jobject glView)
    {
        const ScopedLock sl (contextListLock);

        for (int i = contextList.size(); --i >= 0;)
        {
            NativeContext* const c = contextList.getUnchecked(i);

            if (env->IsSameObject (c->glView.get(), glView))
                return c;
        }

        return nullptr;
    }

    static NativeContext* getActiveContext() noexcept
    {
        const ScopedLock sl (contextListLock);

        for (int i = contextList.size(); --i >= 0;)
        {
            NativeContext* const c = contextList.getUnchecked(i);

            if (c->isInsideGLCallback)
                return c;
        }

        return nullptr;
    }

    struct Locker { Locker (NativeContext&) {} };

    Component& component;

private:
    GlobalRef glView;
    Rectangle<int> lastBounds;
    bool isInsideGLCallback;

    typedef Array<NativeContext*> ContextArray;
    static CriticalSection contextListLock;
    static ContextArray contextList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeContext)
};

CriticalSection OpenGLContext::NativeContext::contextListLock;
OpenGLContext::NativeContext::ContextArray OpenGLContext::NativeContext::contextList;

//==============================================================================
bool OpenGLHelpers::isContextActive()
{
    return OpenGLContext::NativeContext::getActiveContext() != nullptr;
}

//==============================================================================
#define GL_VIEW_CLASS_NAME    JUCE_JOIN_MACRO (JUCE_ANDROID_ACTIVITY_CLASSNAME, _00024OpenGLView)

JUCE_JNI_CALLBACK (GL_VIEW_CLASS_NAME, contextCreated, void, (JNIEnv* env, jobject view))
{
    threadLocalJNIEnvHolder.getOrAttach();

    if (OpenGLContext::NativeContext* const context = OpenGLContext::NativeContext::findContextFor (env, view))
        context->contextCreatedCallback();
    else
        jassertfalse;
}

JUCE_JNI_CALLBACK (GL_VIEW_CLASS_NAME, contextChangedSize, void, (JNIEnv* env, jobject view))
{
    if (OpenGLContext::NativeContext* const context = OpenGLContext::NativeContext::findContextFor (env, view))
        context->contextChangedSize();
}

JUCE_JNI_CALLBACK (GL_VIEW_CLASS_NAME, render, void, (JNIEnv* env, jobject view))
{
    if (OpenGLContext::NativeContext* const context = OpenGLContext::NativeContext::findContextFor (env, view))
        context->renderCallback();
}
