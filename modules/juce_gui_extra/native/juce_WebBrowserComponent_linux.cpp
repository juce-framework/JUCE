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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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

// This type isn't in the headers until v2.36
#if ! WEBKIT_CHECK_VERSION (2, 36, 0)
struct WebKitURISchemeResponse;
#endif

namespace juce
{

//==============================================================================
class WebKitSymbols final : public DeletedAtShutdown
{
public:
    //==============================================================================
    bool isWebKitAvailable() const noexcept  { return webKitIsAvailable; }

    //==============================================================================
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_new, juce_webkit_settings_new,
                                         (), WebKitSettings*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_hardware_acceleration_policy, juce_webkit_settings_set_hardware_acceleration_policy,
                                         (WebKitSettings*, int), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_user_agent, juce_webkit_settings_set_user_agent,
                                         (WebKitSettings*, const gchar*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_new_with_settings, juce_webkit_web_view_new_with_settings,
                                         (WebKitSettings*), GtkWidget*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_load_request, juce_webkit_web_view_load_request,
                                         (WebKitWebView*, const WebKitURIRequest*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_request_new, juce_webkit_uri_request_new,
                                         (const gchar*), WebKitURIRequest*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_request_get_http_headers, juce_webkit_uri_request_get_http_headers,
                                         (WebKitURIRequest*), SoupMessageHeaders*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_policy_decision_use, juce_webkit_policy_decision_use,
                                         (WebKitPolicyDecision*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_policy_decision_ignore, juce_webkit_policy_decision_ignore,
                                         (WebKitPolicyDecision*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_go_back, juce_webkit_web_view_go_back,
                                         (WebKitWebView*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_go_forward, juce_webkit_web_view_go_forward,
                                         (WebKitWebView*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_reload, juce_webkit_web_view_reload,
                                         (WebKitWebView*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_stop_loading, juce_webkit_web_view_stop_loading,
                                         (WebKitWebView*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_request_get_uri, juce_webkit_uri_request_get_uri,
                                         (WebKitURIRequest*), const gchar*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_navigation_action_get_request, juce_webkit_navigation_action_get_request,
                                         (WebKitNavigationAction*), WebKitURIRequest*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_navigation_policy_decision_get_frame_name, juce_webkit_navigation_policy_decision_get_frame_name,
                                         (WebKitNavigationPolicyDecision*), const gchar*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_navigation_policy_decision_get_navigation_action, juce_webkit_navigation_policy_decision_get_navigation_action,
                                         (WebKitNavigationPolicyDecision*), WebKitNavigationAction*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_get_uri, juce_webkit_web_view_get_uri,
                                         (WebKitWebView*), const gchar*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_run_javascript, juce_webkit_web_view_run_javascript,
                                         (WebKitWebView*, const gchar*, GCancellable*, GAsyncReadyCallback, gpointer), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_javascript_result_unref, juce_webkit_javascript_result_unref,
                                         (WebKitJavascriptResult*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_run_javascript_finish, juce_webkit_web_view_run_javascript_finish,
                                         (WebKitWebView*, GAsyncResult*, GError**), WebKitJavascriptResult*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_javascript_result_get_js_value, juce_webkit_javascript_result_get_js_value,
                                         (WebKitJavascriptResult*), JSCValue*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (jsc_value_to_string, juce_jsc_value_to_string,
                                         (JSCValue*), char*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_get_user_content_manager,
                                         juce_webkit_web_view_get_user_content_manager,
                                         (WebKitWebView*), WebKitUserContentManager*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_javascript_can_access_clipboard,
                                         juce_webkit_settings_set_javascript_can_access_clipboard,
                                         (WebKitSettings*, gboolean), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_enable_write_console_messages_to_stdout,
                                         juce_webkit_settings_set_enable_write_console_messages_to_stdout,
                                         (WebKitSettings*, gboolean), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_enable_developer_extras,
                                         juce_webkit_settings_set_enable_developer_extras,
                                         (WebKitSettings*, gboolean), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_user_content_manager_register_script_message_handler,
                                         juce_webkit_user_content_manager_register_script_message_handler,
                                         (WebKitUserContentManager*, const gchar*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_user_script_new,
                                         juce_webkit_user_script_new,
                                         (const gchar*,
                                          WebKitUserContentInjectedFrames,
                                          WebKitUserScriptInjectionTime,
                                          const gchar* const*,
                                          const gchar* const*),
                                         WebKitUserScript*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_user_content_manager_add_script,
                                         juce_webkit_user_content_manager_add_script,
                                         (WebKitUserContentManager*, WebKitUserScript*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_context_register_uri_scheme,
                                         juce_webkit_web_context_register_uri_scheme,
                                         (WebKitWebContext*,
                                          const gchar*,
                                          WebKitURISchemeRequestCallback,
                                          gpointer,
                                          GDestroyNotify),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_get_context,
                                         juce_webkit_web_view_get_context,
                                         (WebKitWebView*),
                                         WebKitWebContext*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_request_get_path,
                                         juce_webkit_uri_scheme_request_get_path,
                                         (WebKitURISchemeRequest*),
                                         const gchar*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_response_new,
                                         juce_webkit_uri_scheme_response_new,
                                         (GInputStream*, gint64),
                                         WebKitURISchemeResponse*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_response_set_http_headers,
                                         juce_webkit_uri_scheme_response_set_http_headers,
                                         (WebKitURISchemeResponse*, SoupMessageHeaders*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_response_set_status,
                                         juce_webkit_uri_scheme_response_set_status,
                                         (WebKitURISchemeResponse*, guint, const gchar*),
                                         void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_request_finish_with_response,
                                         juce_webkit_uri_scheme_request_finish_with_response,
                                         (WebKitURISchemeRequest*, WebKitURISchemeResponse*),
                                         void)

    //==============================================================================
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gtk_init, juce_gtk_init,
                                         (int*, char***), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gtk_plug_new, juce_gtk_plug_new,
                                         (::Window), GtkWidget*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gtk_scrolled_window_new, juce_gtk_scrolled_window_new,
                                         (GtkAdjustment*, GtkAdjustment*), GtkWidget*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gtk_container_add, juce_gtk_container_add,
                                         (GtkContainer*, GtkWidget*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gtk_widget_show_all, juce_gtk_widget_show_all,
                                         (GtkWidget*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gtk_plug_get_id, juce_gtk_plug_get_id,
                                         (GtkPlug*), ::Window)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gtk_main, juce_gtk_main,
                                         (), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gtk_main_quit, juce_gtk_main_quit,
                                         (), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_unix_fd_add, juce_g_unix_fd_add,
                                         (gint, GIOCondition, GUnixFDSourceFunc, gpointer), guint)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_object_ref, juce_g_object_ref,
                                         (gpointer), gpointer)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_object_unref, juce_g_object_unref,
                                         (gpointer), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_memory_input_stream_new, juce_g_memory_input_stream_new,
                                         (), GInputStream*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_memory_input_stream_new_from_bytes, juce_g_memory_input_stream_new_from_bytes,
                                         (GBytes*), GInputStream*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_bytes_new, juce_g_bytes_new,
                                         (gconstpointer, gsize), GBytes*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_bytes_unref, juce_g_bytes_unref,
                                         (GBytes*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_error_free, juce_g_error_free,
                                         (GError*), void)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_signal_connect_data, juce_g_signal_connect_data,
                                         (gpointer, const gchar*, GCallback, gpointer, GClosureNotify, GConnectFlags), gulong)

    //==============================================================================
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (gdk_set_allowed_backends, juce_gdk_set_allowed_backends,
                                         (const char*), void)

    //==============================================================================
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (jsc_value_to_json, juce_jsc_value_to_json,
                                         (JSCValue*, guint), char*)

    //==============================================================================
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (soup_message_headers_new, juce_soup_message_headers_new,
                                         (SoupMessageHeadersType), SoupMessageHeaders*)

    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (soup_message_headers_append, juce_soup_message_headers_append,
                                         (SoupMessageHeaders*, const char*, const char*), void)

    //==============================================================================
    JUCE_GENERATE_FUNCTION_WITH_DEFAULT (g_free, juce_g_free,
                                         (gpointer), void)

    //==============================================================================
    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (WebKitSymbols)

private:
    WebKitSymbols() = default;

    ~WebKitSymbols()
    {
        clearSingletonInstance();
    }

    template <typename FuncPtr>
    struct SymbolBinding
    {
        FuncPtr& func;
        const char* name;
    };

    template <typename FuncPtr>
    SymbolBinding<FuncPtr> makeSymbolBinding (FuncPtr& func, const char* name)
    {
        return { func, name };
    }

    template <typename FuncPtr>
    bool loadSymbols (DynamicLibrary& lib, SymbolBinding<FuncPtr> binding)
    {
        if (auto* func = lib.getFunction (binding.name))
        {
            binding.func = reinterpret_cast<FuncPtr> (func);
            return true;
        }

        return false;
    }

    template <typename FuncPtr, typename... Args>
    bool loadSymbols (DynamicLibrary& lib, SymbolBinding<FuncPtr> binding, Args... args)
    {
        return loadSymbols (lib, binding) && loadSymbols (lib, args...);
    }

    //==============================================================================
    bool loadWebkitSymbols()
    {
        return loadSymbols (webkitLib,
                            makeSymbolBinding (juce_webkit_settings_new,                                         "webkit_settings_new"),
                            makeSymbolBinding (juce_webkit_settings_set_hardware_acceleration_policy,            "webkit_settings_set_hardware_acceleration_policy"),
                            makeSymbolBinding (juce_webkit_settings_set_user_agent,                              "webkit_settings_set_user_agent"),
                            makeSymbolBinding (juce_webkit_web_view_new_with_settings,                           "webkit_web_view_new_with_settings"),
                            makeSymbolBinding (juce_webkit_policy_decision_use,                                  "webkit_policy_decision_use"),
                            makeSymbolBinding (juce_webkit_policy_decision_ignore,                               "webkit_policy_decision_ignore"),
                            makeSymbolBinding (juce_webkit_web_view_go_back,                                     "webkit_web_view_go_back"),
                            makeSymbolBinding (juce_webkit_web_view_go_forward,                                  "webkit_web_view_go_forward"),
                            makeSymbolBinding (juce_webkit_web_view_reload,                                      "webkit_web_view_reload"),
                            makeSymbolBinding (juce_webkit_web_view_stop_loading,                                "webkit_web_view_stop_loading"),
                            makeSymbolBinding (juce_webkit_uri_request_get_uri,                                  "webkit_uri_request_get_uri"),
                            makeSymbolBinding (juce_webkit_web_view_load_request,                                "webkit_web_view_load_request"),
                            makeSymbolBinding (juce_webkit_uri_request_new,                                      "webkit_uri_request_new"),
                            makeSymbolBinding (juce_webkit_uri_request_get_http_headers,                         "webkit_uri_request_get_http_headers"),
                            makeSymbolBinding (juce_webkit_navigation_action_get_request,                        "webkit_navigation_action_get_request"),
                            makeSymbolBinding (juce_webkit_navigation_policy_decision_get_frame_name,            "webkit_navigation_policy_decision_get_frame_name"),
                            makeSymbolBinding (juce_webkit_navigation_policy_decision_get_navigation_action,     "webkit_navigation_policy_decision_get_navigation_action"),
                            makeSymbolBinding (juce_webkit_web_view_get_uri,                                     "webkit_web_view_get_uri"),
                            makeSymbolBinding (juce_webkit_web_view_run_javascript,                              "webkit_web_view_run_javascript"),
                            makeSymbolBinding (juce_webkit_javascript_result_unref,                              "webkit_javascript_result_unref"),
                            makeSymbolBinding (juce_webkit_web_view_get_user_content_manager,                    "webkit_web_view_get_user_content_manager"),
                            makeSymbolBinding (juce_webkit_settings_set_javascript_can_access_clipboard,         "webkit_settings_set_javascript_can_access_clipboard"),
                            makeSymbolBinding (juce_webkit_settings_set_enable_write_console_messages_to_stdout, "webkit_settings_set_enable_write_console_messages_to_stdout"),
                            makeSymbolBinding (juce_webkit_settings_set_enable_developer_extras,                 "webkit_settings_set_enable_developer_extras"),
                            makeSymbolBinding (juce_webkit_user_content_manager_register_script_message_handler, "webkit_user_content_manager_register_script_message_handler"),
                            makeSymbolBinding (juce_webkit_user_script_new,                                      "webkit_user_script_new"),
                            makeSymbolBinding (juce_webkit_user_content_manager_add_script,                      "webkit_user_content_manager_add_script"),
                            makeSymbolBinding (juce_webkit_javascript_result_get_js_value,                       "webkit_javascript_result_get_js_value"),
                            makeSymbolBinding (juce_jsc_value_to_string,                                         "jsc_value_to_string"),
                            makeSymbolBinding (juce_webkit_web_view_run_javascript_finish,                       "webkit_web_view_run_javascript_finish"),
                            makeSymbolBinding (juce_webkit_web_context_register_uri_scheme,                      "webkit_web_context_register_uri_scheme"),
                            makeSymbolBinding (juce_webkit_web_view_get_context,                                 "webkit_web_view_get_context"),
                            makeSymbolBinding (juce_webkit_uri_scheme_request_get_path,                          "webkit_uri_scheme_request_get_path"),
                            makeSymbolBinding (juce_webkit_uri_scheme_response_new,                              "webkit_uri_scheme_response_new"),
                            makeSymbolBinding (juce_webkit_uri_scheme_response_set_http_headers,                 "webkit_uri_scheme_response_set_http_headers"),
                            makeSymbolBinding (juce_webkit_uri_scheme_response_set_status,                       "webkit_uri_scheme_response_set_status"),
                            makeSymbolBinding (juce_webkit_uri_scheme_request_finish_with_response,              "webkit_uri_scheme_request_finish_with_response"));
    }

    bool loadGtkSymbols()
    {
        return loadSymbols (gtkLib,
                            makeSymbolBinding (juce_gtk_init,                             "gtk_init"),
                            makeSymbolBinding (juce_gtk_plug_new,                         "gtk_plug_new"),
                            makeSymbolBinding (juce_gtk_scrolled_window_new,              "gtk_scrolled_window_new"),
                            makeSymbolBinding (juce_gtk_container_add,                    "gtk_container_add"),
                            makeSymbolBinding (juce_gtk_widget_show_all,                  "gtk_widget_show_all"),
                            makeSymbolBinding (juce_gtk_plug_get_id,                      "gtk_plug_get_id"),
                            makeSymbolBinding (juce_gtk_main,                             "gtk_main"),
                            makeSymbolBinding (juce_gtk_main_quit,                        "gtk_main_quit"),
                            makeSymbolBinding (juce_g_unix_fd_add,                        "g_unix_fd_add"),
                            makeSymbolBinding (juce_g_object_ref,                         "g_object_ref"),
                            makeSymbolBinding (juce_g_object_unref,                       "g_object_unref"),
                            makeSymbolBinding (juce_g_bytes_new,                          "g_bytes_new"),
                            makeSymbolBinding (juce_g_bytes_unref,                        "g_bytes_unref"),
                            makeSymbolBinding (juce_g_signal_connect_data,                "g_signal_connect_data"),
                            makeSymbolBinding (juce_gdk_set_allowed_backends,             "gdk_set_allowed_backends"),
                            makeSymbolBinding (juce_g_memory_input_stream_new,            "g_memory_input_stream_new"),
                            makeSymbolBinding (juce_g_memory_input_stream_new_from_bytes, "g_memory_input_stream_new_from_bytes"));
    }

    bool loadJsLibSymbols()
    {
        return loadSymbols (jsLib,
                            makeSymbolBinding (juce_jsc_value_to_json, "jsc_value_to_json"));
    }

    bool loadSoupLibSymbols()
    {
        return loadSymbols (soupLib,
                            makeSymbolBinding (juce_soup_message_headers_new, "soup_message_headers_new"),
                            makeSymbolBinding (juce_soup_message_headers_append, "soup_message_headers_append"));
    }

    bool loadGlibSymbols()
    {
        return loadSymbols (glib,
                            makeSymbolBinding (juce_g_free, "g_free"));
    }

    struct WebKitAndDependencyLibraryNames
    {
        const char* webkitLib;
        const char* jsLib;
        const char* soupLib;
    };

    bool openWebKitAndDependencyLibraries (const WebKitAndDependencyLibraryNames& names)
    {
        if (webkitLib.open (names.webkitLib) && jsLib.open (names.jsLib) && soupLib.open (names.soupLib))
            return true;

        for (auto* l : { &webkitLib, &jsLib, &soupLib })
            l->close();

        return false;
    }

    //==============================================================================
    DynamicLibrary webkitLib, jsLib, soupLib;

    DynamicLibrary gtkLib    { "libgtk-3.so" },
                   glib      { "libglib-2.0.so" };

    const bool webKitIsAvailable =    (   openWebKitAndDependencyLibraries ({ "libwebkit2gtk-4.1.so",
                                                                              "libjavascriptcoregtk-4.1.so",
                                                                              "libsoup-3.0.so" })
                                       || openWebKitAndDependencyLibraries ({ "libwebkit2gtk-4.0.so",
                                                                              "libjavascriptcoregtk-4.0.so",
                                                                              "libsoup-2.4.so" }))
                                   && loadWebkitSymbols()
                                   && loadGtkSymbols()
                                   && loadJsLibSymbols()
                                   && loadSoupLibSymbols()
                                   && loadGlibSymbols();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebKitSymbols)
};

//==============================================================================
extern "C" int juce_gtkWebkitMain (int argc, const char* const* argv);

class CommandReceiver
{
public:
    struct Responder
    {
        virtual ~Responder() = default;

        virtual void handleCommand (const String& cmd, const var& param) = 0;
        virtual void receiverHadError() = 0;
    };

    enum class ReturnAfterMessageReceived
    {
        no,
        yes
    };

    CommandReceiver (Responder* responderToUse, int inputChannelToUse)
        : responder (responderToUse), inChannel (inputChannelToUse)
    {
        setBlocking (inChannel, false);
    }

    static void setBlocking (int fd, bool shouldBlock)
    {
        auto flags = fcntl (fd, F_GETFL);
        fcntl (fd, F_SETFL, (shouldBlock ? (flags & ~O_NONBLOCK)
                                         : (flags | O_NONBLOCK)));
    }

    int getFd() const     { return inChannel; }

    void tryNextRead (ReturnAfterMessageReceived ret = ReturnAfterMessageReceived::no)
    {
        for (;;)
        {
            char lengthBytes[sizeof (size_t)]{};
            const auto numLengthBytes = readIntoBuffer (lengthBytes);

            if (numLengthBytes != std::size (lengthBytes))
                break;

            const auto numBytesExpected = readUnaligned<size_t> (lengthBytes);
            buffer.reserve (numBytesExpected + 1);
            buffer.resize (numBytesExpected);

            if (readIntoBuffer (buffer) != numBytesExpected)
                break;

            buffer.push_back (0);
            parseJSON (StringRef (buffer.data()));

            if (ret == ReturnAfterMessageReceived::yes)
                return;
        }

        if (errno != EAGAIN && errno != EWOULDBLOCK && responder != nullptr)
            responder->receiverHadError();
    }

    static void sendCommand (int outChannel, const String& cmd, const var& params)
    {
        DynamicObject::Ptr obj = new DynamicObject;

        obj->setProperty (getCmdIdentifier(), cmd);

        if (! params.isVoid())
            obj->setProperty (getParamIdentifier(), params);

        auto json = JSON::toString (var (obj.get()));

        auto jsonLength = static_cast<size_t> (json.length());
        auto len        = sizeof (size_t) + jsonLength;

        HeapBlock<char> buffer (len);
        auto* dst = buffer.getData();

        memcpy (dst, &jsonLength, sizeof (size_t));
        dst += sizeof (size_t);

        memcpy (dst, json.toRawUTF8(), jsonLength);

        ssize_t ret;

        for (;;)
        {
            ret = write (outChannel, buffer.getData(), len);

            if (ret != -1 || errno != EINTR)
                break;
        }
    }

private:
    void parseJSON (StringRef json)
    {
        auto object = JSON::fromString (json);

        if (! object.isVoid())
        {
            auto cmd    = object.getProperty (getCmdIdentifier(),   {}).toString();
            auto params = object.getProperty (getParamIdentifier(), {});

            if (responder != nullptr)
                responder->handleCommand (cmd, params);
        }
    }

    /*  Try to fill the target buffer by reading from the input channel.
        Returns the number of bytes that were successfully read.
    */
    size_t readIntoBuffer (Span<char> target) const
    {
        size_t pos = 0;

        while (pos != target.size())
        {
            const auto bytesThisTime = read (inChannel, target.data() + pos, target.size() - pos);

            if (bytesThisTime <= 0)
            {
                if (bytesThisTime != 0 && errno == EINTR)
                    continue;

                break;
            }

            pos += static_cast<size_t> (bytesThisTime);
        }

        return pos;
    }

    static Identifier getCmdIdentifier()    { static Identifier Id ("cmd");    return Id; }
    static Identifier getParamIdentifier()  { static Identifier Id ("params"); return Id; }

    std::vector<char> buffer;
    Responder* responder = nullptr;
    int inChannel = 0;
};

#define juce_g_signal_connect(instance, detailed_signal, c_handler, data) \
    WebKitSymbols::getInstance()->juce_g_signal_connect_data (instance, detailed_signal, c_handler, data, nullptr, (GConnectFlags) 0)

static constexpr const char* platformSpecificIntegrationScript = R"(
window.__JUCE__ = {
  postMessage: function (object) {
    window.webkit.messageHandlers.__JUCE__.postMessage(object);
  },
};
)";

struct InitialisationData
{
    bool nativeIntegrationsEnabled;
    String userAgent;
    String userScript;
    String allowedOrigin;

    static constexpr std::optional<int> marshallingVersion = std::nullopt;

    template <typename Archive, typename Item>
    static void serialise (Archive& archive, Item& item)
    {
        archive (named ("nativeIntegrationsEnabled", item.nativeIntegrationsEnabled),
                 named ("userAgent", item.userAgent),
                 named ("userScript", item.userScript),
                 named ("allowedOrigin", item.allowedOrigin));
    }
};

struct EvaluateJavascriptParams
{
    String script;
    bool requireCallback;

    static constexpr std::optional<int> marshallingVersion = std::nullopt;

    template <typename Archive, typename Item>
    static void serialise (Archive& archive, Item& item)
    {
        archive (named ("script", item.script),
                 named ("requireCallback", item.requireCallback));
    }
};

struct EvaluateJavascriptCallbackParams
{
    bool success;

    // This is necessary because a DynamicObject with a property value of var::undefined()
    // cannot be unserialised. So we need to signal this case with an extra variable.
    bool hasPayload;

    var payload;
    String error;

    static constexpr std::optional<int> marshallingVersion = std::nullopt;

    template <typename Archive, typename Item>
    static void serialise (Archive& archive, Item& item)
    {
        archive(named ("success", item.success),
                named ("hasPayload", item.hasPayload),
                named ("payload", item.payload),
                named ("error", item.error));
    }

    static inline String key { "evaluateJavascriptCallbackParams" };
};

struct ResourceRequest
{
    int64 requestId;
    String path;

    static constexpr std::optional<int> marshallingVersion = std::nullopt;

    template <typename Archive, typename Item>
    static void serialise (Archive& archive, Item& item)
    {
        archive (named ("requestId", item.requestId),
                 named ("path", item.path));
    }

    static inline const String key { "resourceRequest" };
};

template <>
struct SerialisationTraits<WebBrowserComponent::Resource>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& item)
    {
        archive (named ("data", item.data),
                 named ("mimeType", item.mimeType));
    }
};

struct ResourceRequestResponse
{
    int64 requestId;
    std::optional<WebBrowserComponent::Resource> resource;

    static constexpr std::optional<int> marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static void serialise (Archive& archive, T& item)
    {
        archive (named ("requestId", item.requestId),
                 named ("resource", item.resource));
    }

    static inline const String key { "resourceRequestResponse" };
};

//==============================================================================
class GtkChildProcess final : private CommandReceiver::Responder
{
public:
    //==============================================================================
    GtkChildProcess (int inChannel, int outChannelToUse, const String& userAgentToUse)
        : outChannel (outChannelToUse),
          receiver (this, inChannel),
          userAgent (userAgentToUse)
    {}

    int entry()
    {
        CommandReceiver::setBlocking (outChannel, true);

        {
            const ScopeGuard scope { [this] { CommandReceiver::setBlocking (receiver.getFd(), false); } };
            CommandReceiver::setBlocking (receiver.getFd(), true);
            receiver.tryNextRead (CommandReceiver::ReturnAfterMessageReceived::yes);

            if (! initialisationData.has_value())
            {
                std::cerr << "The first message received by GtkChildProcess should have been "
                             "the initialisationData, but it wasn't." << std::endl;

                return 1;
            }
        }

        auto& wk = *WebKitSymbols::getInstance();

        // webkit2gtk crashes when using the wayland backend embedded into an x11 window
        wk.juce_gdk_set_allowed_backends ("x11");

        wk.juce_gtk_init (nullptr, nullptr);

        auto* settings = wk.juce_webkit_settings_new();

        static constexpr int webkitHadwareAccelerationPolicyNeverFlag = 2;

        WebKitSymbols::getInstance()->juce_webkit_settings_set_hardware_acceleration_policy (settings,
                                                                                             webkitHadwareAccelerationPolicyNeverFlag);
        if (initialisationData->userAgent.isNotEmpty())
            WebKitSymbols::getInstance()->juce_webkit_settings_set_user_agent (settings,
                                                                               initialisationData->userAgent.toRawUTF8());

        auto* plug      = WebKitSymbols::getInstance()->juce_gtk_plug_new (0);
        auto* container = WebKitSymbols::getInstance()->juce_gtk_scrolled_window_new (nullptr, nullptr);

       #if JUCE_DEBUG
        wk.juce_webkit_settings_set_enable_write_console_messages_to_stdout (settings, true);
        wk.juce_webkit_settings_set_enable_developer_extras (settings, true);
       #endif

        auto* webviewWidget = WebKitSymbols::getInstance()->juce_webkit_web_view_new_with_settings (settings);
        webview = (WebKitWebView*) webviewWidget;

        if (initialisationData->nativeIntegrationsEnabled)
        {
            manager = wk.juce_webkit_web_view_get_user_content_manager (webview);

            // It's probably fine to not disconnect these signals, given that upon closing the
            // WebBrowserComponent the entire subprocess is cleaned up with the manager and
            // everything.
            juce_g_signal_connect (manager,
                                   "script-message-received::__JUCE__",
                                   G_CALLBACK (+[] (WebKitUserContentManager*, WebKitJavascriptResult* r, gpointer arg)
                                   {
                                       static_cast<GtkChildProcess*> (arg)->invokeCallback (r);
                                   }),
                                   this);

            wk.juce_webkit_user_content_manager_register_script_message_handler (manager, "__JUCE__");

            auto* context = wk.juce_webkit_web_view_get_context (webview);
            wk.juce_webkit_web_context_register_uri_scheme (context, "juce", resourceRequestedCallback, this, nullptr);

            const StringArray userScripts { platformSpecificIntegrationScript,
                                            initialisationData->userScript };

            wk.juce_webkit_user_content_manager_add_script (manager, wk.juce_webkit_user_script_new (userScripts.joinIntoString ("\n").toRawUTF8(),
                                                                                                           WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                                                                                                           WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
                                                                                                           nullptr, nullptr));
        }

        WebKitSymbols::getInstance()->juce_gtk_container_add ((GtkContainer*) container, webviewWidget);
        WebKitSymbols::getInstance()->juce_gtk_container_add ((GtkContainer*) plug,      container);

        goToURLWithHeaders ("about:blank", {});

        juce_g_signal_connect (webview, "decide-policy",
                               (GCallback) decidePolicyCallback, this);

        juce_g_signal_connect (webview, "load-changed",
                               (GCallback) loadChangedCallback, this);

        juce_g_signal_connect (webview, "load-failed",
                               (GCallback) loadFailedCallback, this);

        WebKitSymbols::getInstance()->juce_gtk_widget_show_all (plug);
        auto wID = (unsigned long) WebKitSymbols::getInstance()->juce_gtk_plug_get_id ((GtkPlug*) plug);

        ssize_t ret;

        for (;;)
        {
            ret = write (outChannel, &wID, sizeof (wID));

            if (ret != -1 || errno != EINTR)
                break;
        }

        WebKitSymbols::getInstance()->juce_g_unix_fd_add (receiver.getFd(), G_IO_IN, pipeReadyStatic, this);
        receiver.tryNextRead();

        WebKitSymbols::getInstance()->juce_gtk_main();

        WebKitSymbols::getInstance()->deleteInstance();
        return 0;
    }

    void invokeCallback (WebKitJavascriptResult* r)
    {
        auto& wk = *WebKitSymbols::getInstance();

        auto s = wk.juce_jsc_value_to_string (wk.juce_webkit_javascript_result_get_js_value (r));
        CommandReceiver::sendCommand (outChannel, "invokeCallback", var (s));
        wk.juce_g_free (s);
    }

    void goToURLWithHeaders (StringRef url, Span<const var> headers)
    {
        auto& wk = *WebKitSymbols::getInstance();

        auto* request = wk.juce_webkit_uri_request_new (url.text.getAddress());
        const ScopeGuard requestScope { [&] { wk.juce_g_object_unref (request); } };

        if (! headers.empty())
        {
            if (auto* soupHeaders = wk.juce_webkit_uri_request_get_http_headers (request))
            {
                for (const String item : headers)
                {
                    const auto key   = item.upToFirstOccurrenceOf (":", false, false);
                    const auto value = item.fromFirstOccurrenceOf (":", false, false);

                    if (key.isNotEmpty() && value.isNotEmpty())
                        wk.juce_soup_message_headers_append (soupHeaders, key.toRawUTF8(), value.toRawUTF8());
                    else
                        jassertfalse; // malformed headers?
                }
            }
        }

        wk.juce_webkit_web_view_load_request (webview, request);
    }

    void goToURL (const var& params)
    {
        static const Identifier urlIdentifier ("url");
        const String url = params[urlIdentifier];

        if (url.isEmpty())
            return;

        static const Identifier headersIdentifier ("headers");
        const auto* headers = params[headersIdentifier].getArray();

        static const Identifier postDataIdentifier ("postData");
        [[maybe_unused]] const auto* postData = params[postDataIdentifier].getBinaryData();
        // post data is not currently sent
        jassert (postData == nullptr);

        goToURLWithHeaders (url,
                            headers != nullptr ? Span { headers->getRawDataPointer(), (size_t) headers->size() }
                                               : Span<const var>{});
    }

    void handleDecisionResponse (const var& params)
    {
        auto* decision = (WebKitPolicyDecision*) ((int64) params.getProperty ("decision_id", var (0)));
        bool allow = params.getProperty ("allow", var (false));

        if (decision != nullptr && decisions.contains (decision))
        {
            if (allow)
                WebKitSymbols::getInstance()->juce_webkit_policy_decision_use (decision);
            else
                WebKitSymbols::getInstance()->juce_webkit_policy_decision_ignore (decision);

            decisions.removeAllInstancesOf (decision);
            WebKitSymbols::getInstance()->juce_g_object_unref (decision);
        }
    }

    void evaluateJavascript (const var& params)
    {
        const auto jsParams = FromVar::convert<EvaluateJavascriptParams> (params);

        if (! jsParams.has_value())
        {
            std::cerr << "Wrong params received by evaluateJavascript()" << std::endl;
            return;
        }

        WebKitSymbols::getInstance()->juce_webkit_web_view_run_javascript (webview,
                                                                           jsParams->script.toRawUTF8(),
                                                                           nullptr,
                                                                           javascriptFinishedCallback,
                                                                           this);
    }

    void handleResourceRequestedResponse (const var& params)
    {
        auto& wk = *WebKitSymbols::getInstance();

        const auto response = FromVar::convert<ResourceRequestResponse> (params);

        if (! response.has_value())
        {
            std::cerr << "Bad request response received" << std::endl;
            return;
        }

        auto* request = requestIds.remove (response->requestId);

        // The WebKitURISchemeResponse object will take ownership of the headers
        auto* headers = wk.juce_soup_message_headers_new (SoupMessageHeadersType::SOUP_MESSAGE_HEADERS_RESPONSE);

        if (initialisationData->allowedOrigin.isNotEmpty())
            wk.juce_soup_message_headers_append (headers, "Access-Control-Allow-Origin", initialisationData->allowedOrigin.toRawUTF8());

        if (response->resource.has_value())
        {
            auto* streamBytes = wk.juce_g_bytes_new (response->resource->data.data(),
                                                        static_cast<gsize> (response->resource->data.size()));
            ScopeGuard bytesScope { [&] { wk.juce_g_bytes_unref (streamBytes); } };

            auto* stream = wk.juce_g_memory_input_stream_new_from_bytes (streamBytes);
            ScopeGuard streamScope { [&] { wk.juce_g_object_unref (stream); } };

            auto* webkitResponse = wk.juce_webkit_uri_scheme_response_new (stream,
                                                                              static_cast<gint64> (response->resource->data.size()));
            ScopeGuard webkitResponseScope { [&] { wk.juce_g_object_unref (webkitResponse); } };

            wk.juce_soup_message_headers_append (headers, "Content-Type", response->resource->mimeType.toRawUTF8());

            wk.juce_webkit_uri_scheme_response_set_http_headers (webkitResponse, headers);
            wk.juce_webkit_uri_scheme_response_set_status (webkitResponse, 200, nullptr);
            wk.juce_webkit_uri_scheme_request_finish_with_response (request, webkitResponse);

            return;
        }

        auto* stream = wk.juce_g_memory_input_stream_new();
        ScopeGuard streamScope { [&] { wk.juce_g_object_unref (stream); } };

        auto* webkitResponse = wk.juce_webkit_uri_scheme_response_new (stream, 0);
        ScopeGuard webkitResponseScope { [&] { wk.juce_g_object_unref (webkitResponse); } };

        wk.juce_webkit_uri_scheme_response_set_http_headers (webkitResponse, headers);
        wk.juce_webkit_uri_scheme_response_set_status (webkitResponse, 404, nullptr);
        wk.juce_webkit_uri_scheme_request_finish_with_response (request, webkitResponse);
    }

    //==============================================================================
    void handleCommand (const String& cmd, const var& params) override
    {
        auto& wk = *WebKitSymbols::getInstance();

        if      (cmd == "quit")                       quit();
        else if (cmd == "goToURL")                    goToURL (params);
        else if (cmd == "goBack")                     wk.juce_webkit_web_view_go_back      (webview);
        else if (cmd == "goForward")                  wk.juce_webkit_web_view_go_forward   (webview);
        else if (cmd == "refresh")                    wk.juce_webkit_web_view_reload       (webview);
        else if (cmd == "stop")                       wk.juce_webkit_web_view_stop_loading (webview);
        else if (cmd == "decision")                   handleDecisionResponse (params);
        else if (cmd == "init")                       initialisationData = FromVar::convert<InitialisationData> (params);
        else if (cmd == "evaluateJavascript")         evaluateJavascript (params);
        else if (cmd == ResourceRequestResponse::key) handleResourceRequestedResponse (params);
    }

    void receiverHadError() override
    {
        exit (-1);
    }

    //==============================================================================
    bool pipeReady (gint fd, GIOCondition)
    {
        if (fd == receiver.getFd())
        {
            receiver.tryNextRead();
            return true;
        }

        return false;
    }

    void quit()
    {
        WebKitSymbols::getInstance()->juce_gtk_main_quit();
    }

    String getURIStringForAction (WebKitNavigationAction* action)
    {
        auto* request = WebKitSymbols::getInstance()->juce_webkit_navigation_action_get_request (action);
        return WebKitSymbols::getInstance()->juce_webkit_uri_request_get_uri (request);
    }

    bool onNavigation (String frameName,
                       WebKitNavigationAction* action,
                       WebKitPolicyDecision* decision)
    {
        if (decision != nullptr && frameName.isEmpty())
        {
            WebKitSymbols::getInstance()->juce_g_object_ref (decision);
            decisions.add (decision);

            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", getURIStringForAction (action));
            params->setProperty ("decision_id", (int64) decision);
            CommandReceiver::sendCommand (outChannel, "pageAboutToLoad", var (params.get()));

            return true;
        }

        return false;
    }

    bool onNewWindow (String /*frameName*/,
                      WebKitNavigationAction* action,
                      WebKitPolicyDecision* decision)
    {
        if (decision != nullptr)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", getURIStringForAction (action));
            CommandReceiver::sendCommand (outChannel, "newWindowAttemptingToLoad", var (params.get()));

            // never allow new windows
            WebKitSymbols::getInstance()->juce_webkit_policy_decision_ignore (decision);

            return true;
        }

        return false;
    }

    void onLoadChanged (WebKitLoadEvent loadEvent)
    {
        if (loadEvent == WEBKIT_LOAD_FINISHED)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", String (WebKitSymbols::getInstance()->juce_webkit_web_view_get_uri (webview)));
            CommandReceiver::sendCommand (outChannel, "pageFinishedLoading", var (params.get()));
        }
    }

    bool onDecidePolicy (WebKitPolicyDecision*    decision,
                         WebKitPolicyDecisionType decisionType)
    {
        switch (decisionType)
        {
        case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
            {
                auto* navigationDecision = (WebKitNavigationPolicyDecision*) decision;
                auto* frameName = WebKitSymbols::getInstance()->juce_webkit_navigation_policy_decision_get_frame_name (navigationDecision);

                return onNavigation (String (frameName != nullptr ? frameName : ""),
                                     WebKitSymbols::getInstance()->juce_webkit_navigation_policy_decision_get_navigation_action (navigationDecision),
                                     decision);
            }
            break;
        case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
            {
                auto* navigationDecision = (WebKitNavigationPolicyDecision*) decision;
                auto* frameName = WebKitSymbols::getInstance()->juce_webkit_navigation_policy_decision_get_frame_name (navigationDecision);

                return onNewWindow  (String (frameName != nullptr ? frameName : ""),
                                     WebKitSymbols::getInstance()->juce_webkit_navigation_policy_decision_get_navigation_action (navigationDecision),
                                     decision);
            }
            break;
        case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
            {
                [[maybe_unused]] auto* response = (WebKitNavigationPolicyDecision*) decision;

                // for now just always allow response requests
                WebKitSymbols::getInstance()->juce_webkit_policy_decision_use (decision);
                return true;
            }
            break;
        default:
            break;
        }

        return false;
    }

    void onLoadFailed (GError* error)
    {
        DynamicObject::Ptr params = new DynamicObject;

        params->setProperty ("error", String (error != nullptr ? error->message : "unknown error"));
        CommandReceiver::sendCommand (outChannel, "pageLoadHadNetworkError", var (params.get()));
    }

private:
    void handleEvaluationCallback (const std::optional<var>& value, const String& error)
    {
        const auto success = value.has_value();
        const auto hasPayload = success && ! value->isUndefined();

        CommandReceiver::sendCommand (outChannel,
                                      EvaluateJavascriptCallbackParams::key,
                                      *ToVar::convert (EvaluateJavascriptCallbackParams { success,
                                                                                          hasPayload,
                                                                                          hasPayload ? *value : var{},
                                                                                          error }));
    }

    void handleResourceRequestedCallback (WebKitURISchemeRequest* request, const String& path)
    {
        const auto requestId = requestIds.insert (request);
        CommandReceiver::sendCommand (outChannel,
                                      ResourceRequest::key,
                                      *ToVar::convert (ResourceRequest { requestId, path }));
    }

    static gboolean pipeReadyStatic (gint fd, GIOCondition condition, gpointer user)
    {
        return (reinterpret_cast<GtkChildProcess*> (user)->pipeReady (fd, condition) ? TRUE : FALSE);
    }

    static gboolean decidePolicyCallback (WebKitWebView*,
                                          WebKitPolicyDecision*    decision,
                                          WebKitPolicyDecisionType decisionType,
                                          gpointer user)
    {
        auto& owner = *reinterpret_cast<GtkChildProcess*> (user);
        return (owner.onDecidePolicy (decision, decisionType) ? TRUE : FALSE);
    }

    static void loadChangedCallback (WebKitWebView*,
                                     WebKitLoadEvent loadEvent,
                                     gpointer        user)
    {
        auto& owner = *reinterpret_cast<GtkChildProcess*> (user);
        owner.onLoadChanged (loadEvent);
    }

    static void loadFailedCallback (WebKitWebView*,
                                    WebKitLoadEvent /*loadEvent*/,
                                    gchar*          /*failing_uri*/,
                                    GError*         error,
                                    gpointer        user)
    {
        auto& owner = *reinterpret_cast<GtkChildProcess*> (user);
        owner.onLoadFailed (error);
    }

    static var fromJSCValue (JSCValue* value)
    {
        auto* json = WebKitSymbols::getInstance()->juce_jsc_value_to_json (value, 0);
        ScopeGuard jsonFreeGuard { [&json]
                                   {
                                       if (json != nullptr)
                                           WebKitSymbols::getInstance()->juce_g_free (json);
                                   } };

        if (json == nullptr)
            return var::undefined();

        return JSON::fromString (CharPointer_UTF8 { json });
    }

    struct WebKitJavascriptResultDeleter
    {
        void operator() (WebKitJavascriptResult* ptr) const noexcept
        {
            if (ptr != nullptr)
                WebKitSymbols::getInstance()->juce_webkit_javascript_result_unref (ptr);
        }
    };

    using WebKitJavascriptResultUniquePtr = std::unique_ptr<WebKitJavascriptResult, WebKitJavascriptResultDeleter>;

    static void javascriptFinishedCallback (GObject*, GAsyncResult* result, gpointer user)
    {
        auto& wk = *WebKitSymbols::getInstance();

        GError* error = nullptr;
        ScopeGuard errorFreeGuard { [&error, &wk]
                                    {
                                        if (error != nullptr)
                                            wk.juce_g_error_free (error);
                                    } };

        auto* owner = reinterpret_cast<GtkChildProcess*> (user);

        // Using the non-deprecated webkit_javascript_result_get_js_value() functions seems easier
        // but returned values fail the JS_IS_VALUE() internal assertion. The example code from the
        // documentation doesn't seem to work either.
        JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS
        WebKitJavascriptResultUniquePtr jsResult { wk.juce_webkit_web_view_run_javascript_finish (owner->webview,
                                                                                                     result,
                                                                                                     &error) };
        JUCE_END_IGNORE_DEPRECATION_WARNINGS

        if (jsResult == nullptr)
        {
            owner->handleEvaluationCallback (std::nullopt,
                                             error != nullptr ? String { CharPointer_UTF8 { error->message } }
                                                              : String{});

            return;
        }

        const auto jsValueResult = [&]() -> std::tuple<std::optional<var>, String>
        {
            auto* jsValue = wk.juce_webkit_javascript_result_get_js_value (jsResult.get());

            if (jsValue == nullptr)
                return { std::nullopt, String{} };

            return { fromJSCValue (jsValue), String{} };
        }();

        owner->handleEvaluationCallback (std::get<0> (jsValueResult), std::get<1> (jsValueResult));
    }

    static void resourceRequestedCallback (WebKitURISchemeRequest* request, gpointer user)
    {
        String path { CharPointer_UTF8 { WebKitSymbols::getInstance()->juce_webkit_uri_scheme_request_get_path (request) } };
        reinterpret_cast<GtkChildProcess*> (user)->handleResourceRequestedCallback (request, path);
    }

    class RequestIds
    {
    public:
        int64 insert (WebKitURISchemeRequest* request)
        {
            const auto requestId = nextRequestId++;

            if (nextRequestId == std::numeric_limits<int64>::max())
                nextRequestId = 0;

            requests[requestId] = request;
            return requestId;
        }

        WebKitURISchemeRequest* remove (int64 requestId)
        {
            auto it = requests.find (requestId);

            if (it == requests.end())
            {
                std::cerr << "Outstanding request not found for id " << requestId << std::endl;
                return nullptr;
            }

            auto r = it->second;
            requests.erase (it);

            return r;
        }

    private:
        std::map<int64, WebKitURISchemeRequest*> requests;
        int64 nextRequestId = 0;
    };

    int outChannel = 0;
    CommandReceiver receiver;
    String userAgent;
    WebKitWebView* webview = nullptr;
    Array<WebKitPolicyDecision*> decisions;
    WebKitUserContentManager* manager = nullptr;
    std::optional<InitialisationData> initialisationData;
    RequestIds requestIds;
};

//==============================================================================
struct WebBrowserComponent::Impl::Platform  : public PlatformInterface,
                                              private Thread,
                                              private CommandReceiver::Responder
{
public:
    Platform (WebBrowserComponent& browserIn,
              const WebBrowserComponent::Options& optionsIn,
              const StringArray& userStrings)
        : Thread (SystemStats::getJUCEVersion() + ": Webview"), browser (browserIn), userAgent (optionsIn.getUserAgent())
    {
        webKitIsAvailable = WebKitSymbols::getInstance()->isWebKitAvailable();
        init (InitialisationData { optionsIn.getNativeIntegrationsEnabled(),
                                   userAgent,
                                   userStrings.joinIntoString ("\n"),
                                   optionsIn.getAllowedOrigin() ? *optionsIn.getAllowedOrigin() : "" });
    }

    ~Platform() override
    {
        quit();
    }

    void fallbackPaint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }

    void evaluateJavascript (const String& script, WebBrowserComponent::EvaluationCallback callback) override
    {
        if (callback != nullptr)
            evaluationCallbacks.push_back (std::move (callback));

        CommandReceiver::sendCommand (outChannel,
                                      "evaluateJavascript",
                                      *ToVar::convert (EvaluateJavascriptParams { script, callback != nullptr }));
    }

    void handleJavascriptEvaluationCallback (const var& paramsIn)
    {
        const auto params = FromVar::convert<EvaluateJavascriptCallbackParams> (paramsIn);

        if (! params.has_value() || evaluationCallbacks.size() == 0)
        {
            jassertfalse;
            return;
        }

        const auto result = [&]
        {
            using Error = EvaluationResult::Error;

            if (! params->success)
            {
                if (params->error.isNotEmpty())
                    return EvaluationResult { Error { Error::Type::javascriptException, params->error } };

                return EvaluationResult { Error { Error::Type::unknown, {} } };
            }

            return EvaluationResult { params->hasPayload ? params->payload : var::undefined() };
        }();

        auto& cb = evaluationCallbacks.front();
        cb (result);
        evaluationCallbacks.pop_front();
    }

    void handleResourceRequest (const var& paramsIn)
    {
        const auto params = FromVar::convert<ResourceRequest> (paramsIn);

        if (! params.has_value())
        {
            jassertfalse;
            return;
        }

        const auto response = browser.impl->handleResourceRequest (params->path);

        CommandReceiver::sendCommand (outChannel,
                                      ResourceRequestResponse::key,
                                      *ToVar::convert (ResourceRequestResponse { params->requestId, response }));
    }

    void setWebViewSize (int, int) override
    {
        resized();
    }

    void checkWindowAssociation() override
    {
    }

    //==============================================================================
    void init (const InitialisationData& initialisationData)
    {
        if (! webKitIsAvailable)
            return;

        launchChild();

        [[maybe_unused]] auto ret = pipe (threadControl);

        jassert (ret == 0);

        CommandReceiver::setBlocking (inChannel,        true);
        CommandReceiver::setBlocking (outChannel,       true);
        CommandReceiver::setBlocking (threadControl[0], false);
        CommandReceiver::setBlocking (threadControl[1], true);

        CommandReceiver::sendCommand (outChannel, "init", *ToVar::convert (initialisationData));

        unsigned long windowHandle;
        auto actual = read (inChannel, &windowHandle, sizeof (windowHandle));

        if (actual != (ssize_t) sizeof (windowHandle))
        {
            killChild();
            return;
        }

        receiver.reset (new CommandReceiver (this, inChannel));

        pfds.push_back ({ threadControl[0],  POLLIN, 0 });
        pfds.push_back ({ receiver->getFd(), POLLIN, 0 });

        startThread();

        xembed.reset (new XEmbedComponent (windowHandle));
        browser.addAndMakeVisible (xembed.get());
    }

    void quit()
    {
        if (! webKitIsAvailable)
            return;

        if (isThreadRunning())
        {
            signalThreadShouldExit();

            char ignore = 0;
            ssize_t ret;

            for (;;)
            {
                ret = write (threadControl[1], &ignore, 1);

                if (ret != -1 || errno != EINTR)
                    break;
            }

            waitForThreadToExit (-1);
            receiver = nullptr;
        }

        if (childProcess != 0)
        {
            CommandReceiver::sendCommand (outChannel, "quit", {});
            killChild();
        }
    }

    //==============================================================================
    void goToURL (const String& url, const StringArray* headers, const MemoryBlock* postData) override
    {
        if (! webKitIsAvailable)
            return;

        DynamicObject::Ptr params = new DynamicObject;

        params->setProperty ("url", url);

        if (headers != nullptr)
            params->setProperty ("headers", var (*headers));

        if (postData != nullptr)
            params->setProperty ("postData", var (*postData));

        CommandReceiver::sendCommand (outChannel, "goToURL", var (params.get()));
    }

    void goBack() override    { if (webKitIsAvailable) CommandReceiver::sendCommand (outChannel, "goBack",    {}); }
    void goForward() override { if (webKitIsAvailable) CommandReceiver::sendCommand (outChannel, "goForward", {}); }
    void refresh() override   { if (webKitIsAvailable) CommandReceiver::sendCommand (outChannel, "refresh",   {}); }
    void stop() override      { if (webKitIsAvailable) CommandReceiver::sendCommand (outChannel, "stop",      {}); }

    void resized()
    {
        if (xembed != nullptr)
            xembed->setBounds (browser.getLocalBounds());
    }

private:
    //==============================================================================
    void killChild()
    {
        if (childProcess != 0)
        {
            xembed = nullptr;

            int status = 0, result = 0;

            result = waitpid (childProcess, &status, WNOHANG);
            for (int i = 0; i < 15 && (! WIFEXITED (status) || result != childProcess); ++i)
            {
                Thread::sleep (100);
                result = waitpid (childProcess, &status, WNOHANG);
            }

            // clean-up any zombies
            status = 0;
            if (! WIFEXITED (status) || result != childProcess)
            {
                for (;;)
                {
                    kill (childProcess, SIGTERM);
                    waitpid (childProcess, &status, 0);

                    if (WIFEXITED (status))
                        break;
                }
            }

            childProcess = 0;
        }
    }

    void launchChild()
    {
        int inPipe[2], outPipe[2];

        [[maybe_unused]] auto ret = pipe (inPipe);
        jassert (ret == 0);

        ret = pipe (outPipe);
        jassert (ret == 0);

        std::vector<String> arguments;

       #if JUCE_USE_EXTERNAL_TEMPORARY_SUBPROCESS
        if (! JUCEApplicationBase::isStandaloneApp())
        {
            subprocessFile.emplace ("_juce_linux_subprocess");

            const auto externalSubprocessAvailable = subprocessFile->getFile().replaceWithData (LinuxSubprocessHelperBinaryData::juce_linux_subprocess_helper,
                                                                                                LinuxSubprocessHelperBinaryData::juce_linux_subprocess_helperSize)
                                                     && subprocessFile->getFile().setExecutePermission (true);

            ignoreUnused (externalSubprocessAvailable);
            jassert (externalSubprocessAvailable);

            /*  The external subprocess will load the .so specified as its first argument and execute
                the function specified by the second. The remaining arguments will be passed on to
                the function.
            */
            arguments.emplace_back (subprocessFile->getFile().getFullPathName());
            arguments.emplace_back (File::getSpecialLocation (File::currentExecutableFile).getFullPathName());
            arguments.emplace_back ("juce_gtkWebkitMain");
        }
       #endif

        arguments.emplace_back (File::getSpecialLocation (File::currentExecutableFile).getFullPathName());
        arguments.emplace_back ("--juce-gtkwebkitfork-child");
        arguments.emplace_back (outPipe[0]);
        arguments.emplace_back (inPipe [1]);

        if (userAgent.isNotEmpty())
            arguments.emplace_back (userAgent);

        std::vector<const char*> argv (arguments.size() + 1, nullptr);
        std::transform (arguments.begin(), arguments.end(), argv.begin(), [] (const auto& arg)
        {
            return arg.toRawUTF8();
        });

        auto pid = fork();

        if (pid == 0)
        {
            close (inPipe[0]);
            close (outPipe[1]);

            if (JUCEApplicationBase::isStandaloneApp())
            {
                execv (arguments[0].toRawUTF8(), (char**) argv.data());
            }
            else
            {
               #if JUCE_USE_EXTERNAL_TEMPORARY_SUBPROCESS
                execv (arguments[0].toRawUTF8(), (char**) argv.data());
               #else
                // After a fork in a multithreaded program, the child can only safely call
                // async-signal-safe functions until it calls execv, but if we reached this point
                // then execv won't be called at all! The following call is unsafe, and is very
                // likely to lead to unexpected behaviour.
                jassertfalse;
                juce_gtkWebkitMain ((int) arguments.size(), argv.data());
               #endif
            }

            exit (0);
        }

        close (inPipe[1]);
        close (outPipe[0]);

        inChannel  = inPipe[0];
        outChannel = outPipe[1];

        childProcess = pid;
    }

    void run() override
    {
        while (! threadShouldExit())
        {
            if (shouldExit())
                return;

            receiver->tryNextRead();

            int result = 0;

            while (result == 0 || (result < 0 && errno == EINTR))
                result = poll (&pfds.front(), static_cast<nfds_t> (pfds.size()), 10);

            if (result < 0)
                break;
        }
    }

    bool shouldExit()
    {
        char ignore;
        auto result = read (threadControl[0], &ignore, 1);

        return (result != -1 || (errno != EAGAIN && errno != EWOULDBLOCK));
    }

    //==============================================================================
    void handleCommandOnMessageThread (const String& cmd, const var& params)
    {
        auto url = params.getProperty ("url", var()).toString();

        if      (cmd == "pageAboutToLoad")                      handlePageAboutToLoad (url, params);
        else if (cmd == "pageFinishedLoading")                  browser.pageFinishedLoading (url);
        else if (cmd == "windowCloseRequest")                   browser.windowCloseRequest();
        else if (cmd == "newWindowAttemptingToLoad")            browser.newWindowAttemptingToLoad (url);
        else if (cmd == "pageLoadHadNetworkError")              handlePageLoadHadNetworkError (params);
        else if (cmd == "invokeCallback")                       invokeCallback (params);
        else if (cmd == EvaluateJavascriptCallbackParams::key)  handleJavascriptEvaluationCallback (params);
        else if (cmd == ResourceRequest::key)                   handleResourceRequest (params);
    }

    void invokeCallback (const var& params)
    {
        browser.impl->handleNativeEvent (JSON::fromString (params.toString()));
    }

    void handlePageAboutToLoad (const String& url, const var& inputParams)
    {
        int64 decision_id = inputParams.getProperty ("decision_id", var (0));

        if (decision_id != 0)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("decision_id", decision_id);
            params->setProperty ("allow", browser.pageAboutToLoad (url));

            CommandReceiver::sendCommand (outChannel, "decision", var (params.get()));
        }
    }

    void handlePageLoadHadNetworkError (const var& params)
    {
        String error = params.getProperty ("error", "Unknown error");

        if (browser.pageLoadHadNetworkError (error))
            goToURL (String ("data:text/plain,") + error, nullptr, nullptr);
    }

    void handleCommand (const String& cmd, const var& params) override
    {
        MessageManager::callAsync ([liveness = std::weak_ptr (livenessProbe), this, cmd, params]
                                   {
                                       if (liveness.lock() != nullptr)
                                           handleCommandOnMessageThread (cmd, params);
                                   });
    }

    void receiverHadError() override {}

    //==============================================================================
    bool webKitIsAvailable = false;

    WebBrowserComponent& browser;
    String userAgent;
    std::unique_ptr<CommandReceiver> receiver;
    int childProcess = 0, inChannel = 0, outChannel = 0;
    int threadControl[2];
    std::unique_ptr<XEmbedComponent> xembed;
    std::shared_ptr<int> livenessProbe = std::make_shared<int> (0);
    std::vector<pollfd> pfds;
    std::optional<TemporaryFile> subprocessFile;
    std::deque<EvaluationCallback> evaluationCallbacks;
};

//==============================================================================
auto WebBrowserComponent::Impl::createAndInitPlatformDependentPart (WebBrowserComponent::Impl& impl,
                                                                    const WebBrowserComponent::Options& options,
                                                                    const StringArray& userStrings)
    -> std::unique_ptr<PlatformInterface>
{
    return std::make_unique<Platform> (impl.owner, options, userStrings);
}

void WebBrowserComponent::clearCookies()
{
    // Currently not implemented on linux as WebBrowserComponent currently does not
    // store cookies on linux
    jassertfalse;
}

bool WebBrowserComponent::areOptionsSupported (const Options& options)
{
    return (options.getBackend() == Options::Backend::defaultBackend);
}

extern "C" __attribute__ ((visibility ("default"))) int juce_gtkWebkitMain (int argc, const char* const* argv)
{
    if (argc < 4)
        return -1;

    GtkChildProcess child (String (argv[2]).getIntValue(),
                           String (argv[3]).getIntValue(),
                           argc >= 5 ? String (argv[4]) : String());

    return child.entry();
}

} // namespace juce
