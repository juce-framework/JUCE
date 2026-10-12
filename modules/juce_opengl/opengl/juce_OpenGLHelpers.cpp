/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

template <typename Char>
static auto* findNullTerminator (const Char* ptr)
{
    while (*ptr != 0)
        ++ptr;

    return ptr;
}

static String preprocessShaderPrecisionStatements (String shaderSource)
{
    return shaderSource.replace ("#lowp#",    OpenGLHelpers::isOpenGLES() ? "lowp" : "")
                       .replace ("#mediump#", OpenGLHelpers::isOpenGLES() ? "mediump" : "")
                       .replace ("#highp#",   OpenGLHelpers::isOpenGLES() ? "highp" : "");
}

static OpenGLVersion getOpenGLVersion()
{
    const auto* versionBegin = glGetString (GL_VERSION);

    if (versionBegin == nullptr)
        return {};

    const auto* versionEnd = findNullTerminator (versionBegin);
    const std::string versionString (versionBegin, versionEnd);
    const auto spaceSeparated = StringArray::fromTokens (versionString.c_str(), false);

    for (const auto& token : spaceSeparated)
    {
        const auto pointSeparated = StringArray::fromTokens (token, ".", "");

        const auto major = pointSeparated[0].getIntValue();
        const auto minor = pointSeparated[1].getIntValue();

        if (major != 0)
            return { major, minor };
    }

    return {};
}

static OpenGLProfile getOpenGLProfile()
{
   #if JUCE_OPENGL_ES
    return OpenGLProfile::core;
   #else
    if (OpenGLHelpers::isOpenGLES())
        return OpenGLProfile::core;

    const auto version = getOpenGLVersion();

    if (version <= OpenGLVersion { 2, 1 })
        return OpenGLProfile::compatibility;

    if (version <= OpenGLVersion { 3, 0 })
    {
        GLint flags{};
        glGetIntegerv (GL_CONTEXT_FLAGS, &flags);
        return (flags & (int) GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT) != 0 ? OpenGLProfile::core : OpenGLProfile::compatibility;
    }

    if (version <= OpenGLVersion { 3, 1 })
    {
        return OpenGLHelpers::isExtensionSupported ("GL_ARB_compatibility") ? OpenGLProfile::compatibility : OpenGLProfile::core;
    }

    GLint profile{};
    glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &profile);
    return (profile & (int) GL_CONTEXT_CORE_PROFILE_BIT) != 0 ? OpenGLProfile::core : OpenGLProfile::compatibility;
   #endif
}

#if JUCE_ANDROID || JUCE_LINUX || JUCE_BSD

struct EGLHelpers
{
    EGLHelpers() = delete;

    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")
    static constexpr EGLContext nullContext = EGL_NO_CONTEXT;
    static constexpr EGLDisplay nullDisplay = EGL_NO_DISPLAY;
    static constexpr EGLSurface nullSurface = EGL_NO_SURFACE;
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

    template <typename Traits>
    class ScopedEGLObject
    {
    public:
        using Type = typename Traits::Type;

        ScopedEGLObject() = default;

        ScopedEGLObject (Type obj, EGLDisplay d)
            : object (obj), display (d) {}

        ScopedEGLObject (ScopedEGLObject&& other) noexcept
            : object  (std::exchange (other.object, Type{})),
              display (std::exchange (other.display, nullDisplay)) {}

        ScopedEGLObject& operator= (ScopedEGLObject&& other) noexcept
        {
            ScopedEGLObject { std::move (other) }.swap (*this);
            return *this;
        }

        ~ScopedEGLObject() noexcept
        {
            if (object != Traits::null)
            {
                jassert (display != nullDisplay);
                Traits::destroy(display, object);
            }
        }

        Type get() const { return object; }

        void reset() noexcept
        {
            *this = ScopedEGLObject();
        }

        void swap (ScopedEGLObject& other) noexcept
        {
            std::swap (other.object,  object);
            std::swap (other.display, display);
        }

        bool operator== (std::nullptr_t) const
        {
            return object == Traits::null;
        }

        bool operator!= (std::nullptr_t other) const
        {
            return ! operator== (other);
        }

    private:
        Type object = Traits::null;
        EGLDisplay display = nullDisplay;
    };

    struct TraitsEGLContext
    {
        using Type = EGLContext;
        static constexpr auto null = nullContext;

        static void destroy (EGLDisplay display, Type t)
        {
            eglDestroyContext (display, t);
        }
    };

    struct TraitsEGLSurface
    {
        using Type = EGLSurface;
        static constexpr auto null = nullSurface;

        static void destroy (EGLDisplay display, Type t)
        {
            eglDestroySurface (display, t);
        }
    };

    using PtrEGLContext = ScopedEGLObject<TraitsEGLContext>;
    using PtrEGLSurface = ScopedEGLObject<TraitsEGLSurface>;

    static PtrEGLContext initEGLContext (OpenGLAPI api,
                                         OpenGLVersion version,
                                         [[maybe_unused]] OpenGLProfile profile,
                                         EGLDisplay display,
                                         EGLConfig config,
                                         EGLContext contextToShareWith)
    {
        [[maybe_unused]] const auto didBind = eglBindAPI (api == OpenGLAPI::openGL ? EGL_OPENGL_API : EGL_OPENGL_ES_API);
        // Failed to bind the requested OpenGL API
        jassert (didBind);

        EGLint renderableType{};
        [[maybe_unused]] const auto didGet = eglGetConfigAttrib (display, config, EGL_RENDERABLE_TYPE, &renderableType);
        // Failed to query the supported renderable types
        jassert (didGet);

        const auto versionsToRequest = std::invoke ([&]() -> std::vector<OpenGLVersion>
        {
            if (api == OpenGLAPI::openGL)
                return { version };

            if (version.major == 3 && (renderableType & EGL_OPENGL_ES3_BIT) != 0)
                return { version, { 3, 0 } };

            if (version.major == 2 && (renderableType & EGL_OPENGL_ES2_BIT) != 0)
                return { version, { 2, 0 } };

            // We request a config with at least EGL_OPENGL_ES2_BIT capabilities, so there's
            // no point checking for ES v1 here.
            // ES 3 is backwards compatible with ES 2, so we may as well return an ES 3 context if
            // it's available.
            if ((renderableType & EGL_OPENGL_ES3_BIT) != 0)
                return { { 3, 0 } };

            if ((renderableType & EGL_OPENGL_ES2_BIT) != 0)
                return { { 2, 0 } };

            return { {} };
        });

        constexpr bool addDebugOption[]
        {
           #if JUCE_DEBUG
            true,
           #endif
            false,
        };

        for (const auto& versionToTry : versionsToRequest)
        {
            for (const auto& shouldDebug : addDebugOption)
            {
                std::vector<EGLint> attribs;

                if (versionToTry != OpenGLVersion{})
                {
                    attribs.insert (attribs.end(),
                    {
                        EGL_CONTEXT_MAJOR_VERSION, versionToTry.major,
                        EGL_CONTEXT_MINOR_VERSION, versionToTry.minor,
                    });
                }

                if (shouldDebug)
                {
                    attribs.insert (attribs.end(), { EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE });
                }

               #if ! JUCE_OPENGL_ES
                if (api == OpenGLAPI::openGL)
                {
                   #if JUCE_DEBUG
                    constexpr EGLint contextFlags = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
                   #else
                    constexpr EGLint contextFlags = 0;
                   #endif

                    attribs.insert (attribs.end(),
                    {
                        EGL_CONTEXT_FLAGS_KHR, contextFlags,
                        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
                        profile == OpenGLProfile::core ? EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR
                                                       : EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT_KHR,
                    });
                }
               #endif

                attribs.push_back (EGL_NONE);

                if (PtrEGLContext renderContext { eglCreateContext (display, config, contextToShareWith, attribs.data()), display };
                    renderContext != nullptr)
                {
                    return renderContext;
                }
            }
        }

        return { eglCreateContext (display, config, contextToShareWith, nullptr), display };
    }
};

#endif

void OpenGLHelpers::resetErrorState()
{
    // GL implementations retain one flag per error category, so the total number of errors
    // will be small, but we need to guard against infinite loops in some situations like
    // when the context has been lost.
    for (int i = 0; i < 16; ++i)
        if (glGetError() == GL_NO_ERROR)
            break;
}

void* OpenGLHelpers::getExtensionFunction (const char* functionName)
{
   #if JUCE_WINDOWS
    return (void*) wglGetProcAddress (functionName);
   #elif JUCE_LINUX || JUCE_BSD
    return (void*) eglGetProcAddress (functionName);
   #else
    static void* handle = dlopen (nullptr, RTLD_LAZY);
    return dlsym (handle, functionName);
   #endif
}

bool OpenGLHelpers::isExtensionSupported (const char* const extensionName)
{
    jassert (extensionName != nullptr); // you must supply a genuine string for this
    jassert (isContextActive()); // an OpenGL context will need to be active before calling this

    if (getOpenGLVersion().major >= 3)
    {
        using GetStringi = const GLubyte* (*) (GLenum, GLuint);

        if (auto* thisGlGetStringi = reinterpret_cast<GetStringi> (getExtensionFunction ("glGetStringi")))
        {
            GLint n = 0;
            glGetIntegerv (GL_NUM_EXTENSIONS, &n);

            for (auto i = (decltype (n)) 0; i < n; ++i)
                if (StringRef (extensionName) == StringRef ((const char*) thisGlGetStringi (GL_EXTENSIONS, (GLuint) i)))
                    return true;

            return false;
        }
    }

    const char* extensions = (const char*) glGetString (GL_EXTENSIONS);
    jassert (extensions != nullptr); // Perhaps you didn't activate an OpenGL context before calling this?

    for (;;)
    {
        const char* found = strstr (extensions, extensionName);

        if (found == nullptr)
            break;

        extensions = found + strlen (extensionName);

        if (extensions[0] == ' ' || extensions[0] == 0)
            return true;
    }

    return false;
}

void OpenGLHelpers::clear (Colour colour)
{
    glClearColor (colour.getFloatRed(), colour.getFloatGreen(),
                  colour.getFloatBlue(), colour.getFloatAlpha());

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLHelpers::enableScissorTest (Rectangle<int> clip)
{
    glEnable (GL_SCISSOR_TEST);
    glScissor (clip.getX(), clip.getY(), clip.getWidth(), clip.getHeight());
}

String OpenGLHelpers::getGLSLVersionString()
{
    if (getOpenGLVersion() >= OpenGLVersion (3, 2))
    {
        if (isOpenGLES())
            return "#version 300 es";

        return "#version 150";
    }

    return "#version 110";
}

String OpenGLHelpers::translateVertexShaderToV3 (const String& code)
{
    if (getOpenGLVersion() >= OpenGLVersion (3, 2))
    {
        String output;

       #if JUCE_ANDROID
        {
            int numAttributes = 0;

            for (int p = code.indexOf (0, "attribute "); p >= 0; p = code.indexOf (p + 1, "attribute "))
                numAttributes++;

            int last = 0;

            for (int p = code.indexOf (0, "attribute "); p >= 0; p = code.indexOf (p + 1, "attribute "))
            {
                output += code.substring (last, p) + "layout(location=" + String (--numAttributes) + ") in ";

                last = p + 10;
            }

            output += code.substring (last);
        }
       #else
        output = code.replace ("attribute", "in");
       #endif

        return getGLSLVersionString() + "\n" + output.replace ("varying", "out");
    }

    return code;
}

String OpenGLHelpers::translateFragmentShaderToV3 (const String& code)
{
    if (getOpenGLVersion() >= OpenGLVersion (3, 2))
        return getGLSLVersionString() + "\n"
               "out mediump vec4 fragColor;\n"
                + code.replace ("varying", "in")
                      .replace ("texture2D", "texture")
                      .replace ("gl_FragColor", "fragColor");

    return code;
}

} // namespace juce
