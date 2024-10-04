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

namespace juce
{

#if JUCE_WEB_BROWSER || DOXYGEN

bool WebBrowserComponent::pageAboutToLoad ([[maybe_unused]] const String& newURL)             { return true; }
void WebBrowserComponent::pageFinishedLoading ([[maybe_unused]] const String& url)            {}
bool WebBrowserComponent::pageLoadHadNetworkError ([[maybe_unused]] const String& errorInfo)  { return true; }
void WebBrowserComponent::windowCloseRequest()                                                {}
void WebBrowserComponent::newWindowAttemptingToLoad ([[maybe_unused]] const String& newURL)   {}

// At least this much code has to be injected as user script, since the native backend functions
// rely on the presence of the `window.__JUCE__.backend.emitByBackend()` function. The rest can be
// optionally imported as JS.
static constexpr const char* lowLevelIntegrationsScript = R"(
if (
  typeof window.__JUCE__ !== "undefined" &&
  typeof window.__JUCE__.getAndroidUserScripts !== "undefined" &&
  typeof window.inAndroidUserScriptEval === "undefined"
) {
  window.inAndroidUserScriptEval = true;
  eval(window.__JUCE__.getAndroidUserScripts());
  delete window.inAndroidUserScriptEval;
}

{
  if (typeof window.__JUCE__ === "undefined") {
    console.warn(
      "The 'window.__JUCE__' object is undefined." +
        " Native integration features will not work." +
        " Defining a placeholder 'window.__JUCE__' object."
    );

    window.__JUCE__ = {
      postMessage: function () {},
    };
  }

  if (typeof window.__JUCE__.initialisationData === "undefined") {
    window.__JUCE__.initialisationData = {
      __juce__platform: [],
      __juce__functions: [],
      __juce__registeredGlobalEventIds: [],
      __juce__sliders: [],
      __juce__toggles: [],
      __juce__comboBoxes: [],
    };
  }

  class ListenerList {
    constructor() {
      this.listeners = new Map();
      this.listenerId = 0;
    }

    addListener(fn) {
      const newListenerId = this.listenerId++;
      this.listeners.set(newListenerId, fn);
      return newListenerId;
    }

    removeListener(id) {
      if (this.listeners.has(id)) {
        this.listeners.delete(id);
      }
    }

    callListeners(payload) {
      for (const [, value] of this.listeners) {
        value(payload);
      }
    }
  }

  class EventListenerList {
    constructor() {
      this.eventListeners = new Map();
    }

    addEventListener(eventId, fn) {
      if (!this.eventListeners.has(eventId))
        this.eventListeners.set(eventId, new ListenerList());

      const id = this.eventListeners.get(eventId).addListener(fn);

      return [eventId, id];
    }

    removeEventListener([eventId, id]) {
      if (this.eventListeners.has(eventId)) {
        this.eventListeners.get(eventId).removeListener(id);
      }
    }

    emitEvent(eventId, object) {
      if (this.eventListeners.has(eventId))
        this.eventListeners.get(eventId).callListeners(object);
    }
  }

  class Backend {
    constructor() {
      this.listeners = new EventListenerList();
    }

    addEventListener(eventId, fn) {
      return this.listeners.addEventListener(eventId, fn);
    }

    removeEventListener([eventId, id]) {
      this.listeners.removeEventListener(eventId, id);
    }

    emitEvent(eventId, object) {
      window.__JUCE__.postMessage(
        JSON.stringify({ eventId: eventId, payload: object })
      );
    }

    emitByBackend(eventId, object) {
      this.listeners.emitEvent(eventId, JSON.parse(object));
    }
  }

  if (typeof window.__JUCE__.backend === "undefined")
    window.__JUCE__.backend = new Backend();
}
)";

static void evaluationHandler (WebBrowserComponent::EvaluationResult r)
{
    if (r.getResult() == nullptr)
    {
        // The unsupported return type means a successful Javascript evaluation that yielded a
        // result that cannot be translated and returned to native code such as a Promise.
        jassert (r.getError()->type == WebBrowserComponent::EvaluationResult::Error::Type::unsupportedReturnType);
        DBG (r.getError()->message);
        return;
    }
}

static StringArray getUserScriptsForInitialisationData (const std::vector<std::pair<String, var>>& data)
{
    std::map<String, StringArray> sortedData;

    for (const auto& [key, value] : data)
        sortedData[key].add (JSON::toString (value));

    StringArray result;

    for (const auto& [key, values] : sortedData)
        result.add ("window.__JUCE__.initialisationData." + key + " = ["
                    + values.joinIntoString (",") + "];");

    return result;
}

template <typename T>
static auto getCommaSeparatedList (const T& mapOfStringable)
{
    StringArray keys;

    for (auto it : mapOfStringable)
        keys.add (it.first.toString().quoted());

    return keys.joinIntoString (",");
}

struct NativeEvents
{
    NativeEvents() = delete;

    struct NativeEvent
    {
        String eventId;
        var payload;

        static constexpr std::optional<int> marshallingVersion = std::nullopt;

        template <typename Archive, typename Item>
        static void serialise (Archive& archive, Item& item)
        {
            archive (named ("eventId", item.eventId),
                     named ("payload", item.payload));
        }
    };

    struct Invoke
    {
        String name;
        var params;
        int64 resultId;

        static constexpr std::optional<int> marshallingVersion = std::nullopt;

        template <typename Archive, typename Item>
        static void serialise (Archive& archive, Item& item)
        {
            archive (named ("name", item.name),
                     named ("params", item.params),
                     named ("resultId", item.resultId));
        }

        static inline const Identifier eventId     { "__juce__invoke" };
        static inline const Identifier completeId  { "__juce__complete" };
    };
};

class NativeFunctionsProvider : public OptionsBuilder<WebBrowserComponent::Options>,
                                private AsyncUpdater
{
public:
    explicit NativeFunctionsProvider (WebBrowserComponent& ownerIn)
        : owner (ownerIn)
    {
    }

    WebBrowserComponent::Options buildOptions (const WebBrowserComponent::Options& initialOptions) override
    {
        nativeFunctions = initialOptions.getNativeFunctions();

        if (nativeFunctions.empty())
            return initialOptions;

        auto options = initialOptions.withNativeIntegrationEnabled()
                                     .withEventListener (NativeEvents::Invoke::eventId,
                                                         [this] (const auto& object)
                                                         {
                                                             handleNativeFunctionCall (object);
                                                         });

        for (auto it : nativeFunctions)
            options = options.withInitialisationData ("__juce__functions", it.first.toString());

        return options;
    }

private:
    void handleAsyncUpdate() override
    {
        const auto popFront = [&] (auto&& callback)
        {
            functionCompletionsMutex.lock();
            ErasedScopeGuard unlocker { [&] { functionCompletionsMutex.unlock(); } };

            if (! functionCompletions.empty())
            {
                auto completion = functionCompletions.front();
                functionCompletions.pop_front();
                unlocker.reset();
                callback (completion);
                return true;
            }
            else
            {
                return false;
            }
        };

        while (popFront ([&] (const auto& c) { emitCompletionEvent (c.first, c.second); }))
            ;
    }

    void handleNativeFunctionCall (const var& object)
    {
        const auto invocation = FromVar::convert<NativeEvents::Invoke> (object);

        if (! invocation.has_value())
        {
            jassertfalse;
            return;
        }

        auto it = nativeFunctions.find (invocation->name);

        if (it == nativeFunctions.end())
        {
            jassertfalse;
            return;
        }

        jassert (invocation->params.isArray());

        it->second (*invocation->params.getArray(),
                    [this, resultId = invocation->resultId] (auto result)
                    {
                        completeNativeFunctionCall (resultId, result);
                    });
    }

    void completeNativeFunctionCall (int64 resultId, const var& object)
    {
        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            emitCompletionEvent (resultId, object);
        }
        else
        {
            const std::lock_guard lock { functionCompletionsMutex };
            functionCompletions.push_back (std::make_pair (resultId, object));
            triggerAsyncUpdate();
        }
    }

    void emitCompletionEvent (int64 resultId, const var& object)
    {
        DynamicObject::Ptr eventObject { new DynamicObject() };
        eventObject->setProperty ("promiseId", resultId);
        eventObject->setProperty ("result", object);

        jassert (owner.isVisible());
        owner.emitEventIfBrowserIsVisible (NativeEvents::Invoke::completeId, eventObject.get());
    }

    WebBrowserComponent& owner;
    std::map<Identifier, WebBrowserComponent::NativeFunction> nativeFunctions;
    std::deque<std::pair<int64, var>> functionCompletions;
    std::mutex functionCompletionsMutex;

    JUCE_DECLARE_NON_COPYABLE (NativeFunctionsProvider)
    JUCE_DECLARE_NON_MOVEABLE (NativeFunctionsProvider)
};

class NativeEventListeners
{
public:
    void addListener (const Identifier& eventId, std::function<void (const var&)> handler)
    {
        listeners.push_back (std::move (handler));
        listenerMap[eventId].add (&listeners.back());
    }

    void emit (const Identifier& eventId, const var& object)
    {
        if (const auto& it = listenerMap.find (eventId); it != listenerMap.end())
            it->second.call ([&object] (auto& l) { l (object); });
    }

private:
    std::list<WebBrowserComponent::NativeEventListener> listeners;

    std::map<Identifier, ListenerList<WebBrowserComponent::NativeEventListener>> listenerMap;
};

class WebBrowserComponent::Impl
{
public:
    Impl (WebBrowserComponent& ownerIn, const Options& optionsIn)
        : owner (ownerIn),
          options ([&]
                   {
                       makeFunctionsProviderIfNecessary (nativeFunctionsProvider, *this, optionsIn);

                       if (nativeFunctionsProvider.has_value())
                           return getOptions (optionsIn).withOptionsFrom (*nativeFunctionsProvider);

                       return getOptions (optionsIn);
                   }())
    {
        auto userScripts = options.getUserScripts();

        // Adding user scripts in reverse order of depending on each other. The most depended on
        // comes last.
        for (const auto& eventListener : options.getEventListeners())
        {
            userScripts.insert (0,
                                "window.__JUCE__.initialisationData.__juce__registeredGlobalEventIds = ["
                                   + getCommaSeparatedList (options.getEventListeners()) + "];");

            addPermanentEventListener (eventListener.first, eventListener.second);
        }

        for (const auto& script : getUserScriptsForInitialisationData (options.getInitialisationData()))
            userScripts.insert (0, script);

        userScripts.insert (0, lowLevelIntegrationsScript);

        platform = createAndInitPlatformDependentPart (*this, options, userScripts);
    }

    void emitEvent (const Identifier& eventId, const var& object)
    {
        // The object parameter is serialised into a string and used as a parameter to a Javascript
        // function call. During this JS parameter substitution, control character escape sequences
        // would be interpreted as the control characters themselves. So we need to escape anything
        // that was escaped.
        //
        // We also need to escape the ' character since we use this to delimit the parameter string
        // to emitByBackend.
        const auto objectAsString = JSON::toString (object, true);
        const auto escaped = objectAsString.replace ("\\", "\\\\").replace ("'", "\\'");

        evaluateJavascript ("window.__JUCE__.backend.emitByBackend(" + eventId.toString().quoted() + ", "
                                + escaped.quoted ('\'')
                                + ");", evaluationHandler);
    }

    void goToURL (const String& url, const StringArray* headers, const MemoryBlock* postData)
    {
        platform->goToURL (url, headers, postData);
    }

    void stop()
    {
        platform->stop();
    }

    void goBack()
    {
        platform->goBack();

        owner.lastURL.clear();
        owner.blankPageShown = false;
    }

    void goForward()
    {
        owner.lastURL.clear();

        platform->goForward();
    }

    void refresh()
    {
        platform->refresh();
    }

    void evaluateJavascript (const String& script, EvaluationCallback callback)
    {
        platform->evaluateJavascript (script, std::move (callback));
    }

    void setSize (int width, int height)
    {
        platform->setWebViewSize (width, height);
    }

    void checkWindowAssociation()
    {
        platform->checkWindowAssociation();
    }

    void fallbackPaint (Graphics& webBrowserComponentContext)
    {
        platform->fallbackPaint (webBrowserComponentContext);
    }

    void focusGainedWithDirection (FocusChangeType type, FocusChangeDirection dir)
    {
        platform->focusGainedWithDirection (type, dir);
    }

    struct Platform;

private:
    class PlatformInterface
    {
    public:
        PlatformInterface() = default;
        virtual ~PlatformInterface() = default;

        virtual void goToURL (const String&, const StringArray*, const MemoryBlock*) = 0;
        virtual void goBack() = 0;
        virtual void goForward() = 0;
        virtual void stop() = 0;
        virtual void refresh() = 0;
        virtual void evaluateJavascript (const String&, WebBrowserComponent::EvaluationCallback) = 0;
        virtual void setWebViewSize (int, int) = 0;
        virtual void checkWindowAssociation() = 0;

        virtual void focusGainedWithDirection (FocusChangeType, FocusChangeDirection) {}
        virtual void fallbackPaint (Graphics&) {}
    };

    static void makeFunctionsProviderIfNecessary (std::optional<NativeFunctionsProvider>& provider,
                                                  WebBrowserComponent::Impl& impl,
                                                  const WebBrowserComponent::Options& options)
    {
        if (! options.getNativeFunctions().empty())
            provider.emplace (impl.owner);
    }

    static Options getOptions (const Options& optionsIn)
    {
        const auto os = SystemStats::getOperatingSystemType();

        const auto platformString = [&]
        {
            using OsType = SystemStats::OperatingSystemType;

            if ((os & OsType::MacOSX) != 0)
                return "macos";

            if ((os & OsType::iOS) != 0)
                return "ios";

            if ((os & OsType::Windows) != 0)
                return "windows";

            if ((os & OsType::Android) != 0)
                return "android";

            if ((os & OsType::Linux) != 0)
                return "linux";

            return "";
        }();

        return optionsIn.withInitialisationData ("__juce__platform", platformString);
    }

    void addPermanentEventListener (const Identifier& eventId, NativeEventListener listener)
    {
        nativeEventListeners.addListener (eventId, std::move (listener));
    }

    std::optional<Resource> handleResourceRequest (const String& url)
    {
        if (resourceProvider != nullptr)
            return resourceProvider (url);

        return std::nullopt;
    }

    void handleNativeEvent (const var& message)
    {
        const auto event = FromVar::convert<NativeEvents::NativeEvent> (message);

        if (! event.has_value())
        {
            jassertfalse;
            return;
        }

        nativeEventListeners.emit (event->eventId, event->payload);
    }

    static std::unique_ptr<PlatformInterface> createAndInitPlatformDependentPart (WebBrowserComponent::Impl&,
                                                                                  const Options&,
                                                                                  const StringArray&);

    WebBrowserComponent& owner;
    std::optional<NativeFunctionsProvider> nativeFunctionsProvider;
    Options options;
    ResourceProvider resourceProvider { options.getResourceProvider() };
    NativeEventListeners nativeEventListeners;

    std::unique_ptr<PlatformInterface> platform;
};

//==============================================================================
WebBrowserComponent::WebBrowserComponent (const Options& options)
    : impl (std::make_unique<Impl> (*this, options)),
      unloadPageWhenHidden (! options.keepsPageLoadedWhenBrowserIsHidden())
{
    setOpaque (true);

   #if JUCE_LINUX
    ignoreUnused (blankPageShown);
    ignoreUnused (unloadPageWhenHidden);
   #endif

    for (const auto& l : options.getLifetimeListeners())
        lifetimeListeners.add (l);

    lifetimeListeners.call (&WebViewLifetimeListener::webViewConstructed, this);
}

WebBrowserComponent::~WebBrowserComponent()
{
    lifetimeListeners.call (&WebViewLifetimeListener::webViewDestructed, this);
}

void WebBrowserComponent::emitEventIfBrowserIsVisible (const Identifier& eventId, const var& object)
{
    if (isVisible())
        impl->emitEvent (eventId, object);
}

const String& WebBrowserComponent::getResourceProviderRoot()
{
    const auto os = SystemStats::getOperatingSystemType();

    using OsType = SystemStats::OperatingSystemType;

    if (   (os & OsType::MacOSX) != 0
        || (os & OsType::iOS)    != 0
        || (os & OsType::Linux)  != 0)
    {
        static String backendUrl { "juce://juce.backend/" };
        return backendUrl;
    }

    if (   (os & OsType::Windows) != 0
        || (os & OsType::Android) != 0)
    {
        static String backendUrl { "https://juce.backend/" };
        return backendUrl;
    }

    static String emptyUrl { "" };
    return emptyUrl;
}

void WebBrowserComponent::goToURL (const String& url,
                                   const StringArray* headers,
                                   const MemoryBlock* postData)
{
    lastURL = url;

    if (headers != nullptr)
        lastHeaders = *headers;
    else
        lastHeaders.clear();

    if (postData != nullptr)
        lastPostData = *postData;
    else
        lastPostData.reset();

    impl->goToURL (url, headers, postData);

    blankPageShown = false;
}

void WebBrowserComponent::stop()
{
    impl->stop();
}

void WebBrowserComponent::goBack()
{
    impl->goBack();
    lastURL.clear();
    blankPageShown = false;
}

void WebBrowserComponent::goForward()
{
    lastURL.clear();
    impl->goForward();
}

void WebBrowserComponent::refresh()
{
    impl->refresh();
}

void WebBrowserComponent::paint (Graphics& g)
{
    impl->fallbackPaint (g);
}

void WebBrowserComponent::parentHierarchyChanged()
{
    impl->checkWindowAssociation();
}

void WebBrowserComponent::visibilityChanged()
{
    impl->checkWindowAssociation();
}

void WebBrowserComponent::resized()
{
    impl->setSize (getWidth(), getHeight());
}

void WebBrowserComponent::reloadLastURL()
{
    const auto ptrOrNullIfEmpty = [] (auto& value)
    {
        return ! value.isEmpty() ? &value : nullptr;
    };

    if (lastURL.isNotEmpty())
    {
        goToURL (lastURL, ptrOrNullIfEmpty (lastHeaders), ptrOrNullIfEmpty (lastPostData));
        lastURL.clear();
    }
}

void WebBrowserComponent::evaluateJavascript (const String& script, EvaluationCallback callback)
{
    impl->evaluateJavascript (script, std::move (callback));
}

void WebBrowserComponent::focusGainedWithDirection (FocusChangeType type,
                                                    FocusChangeDirection direction)
{
    impl->focusGainedWithDirection (type, direction);
}

#endif

} // namespace juce
