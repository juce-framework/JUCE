# @juce-framework/webview

JUCE is an open-source cross-platform C++ application framework for creating desktop and mobile applications, including VST, VST3, AU, AUv3, AAX and LV2 audio plug-ins and plug-in hosts.

JUCE supports web based user interfaces through its `WebBrowserComponent`. To enable this there are two-way interfaces between its hosted WebView and the enclosing C++ application.

JavaScript/TypeScript code can
* register listeners for events emitted by the C++ backend,
* emit events that can be observed by the C++ backend,
* call native functions registered on the backend,
* download resources served by the backend.

This allows the creation of self-contained binary distributables that present a web-based UI without external dependencies, such as requiring a separate web server.

Additionally, for plugin projects simple bindings are available to keep sliders, toggles, and combo boxes in your web UI synchronised with `AudioProcessorParameter`s.

For a full example see the WebViewPluginDemo in the JUCE repository.

## Installation

```sh
npm install @juce-framework/webview
```
This package provides TypeScript functions and helper classes to ease the interaction with the native backend provided by the `WebBrowserComponent`.

You need to use a frontend version that matches the one expected by the backend. This is checked by an assertion on the C++ side and a warning log message on the JavaScript side, so you should be notified in case of a mismatch.

It's possible to build and run a frontend project using this package and test it in a browser. There are mock objects defined for this use-case, which allow calling all the backend related APIs, but these mocks won't emit any events, complete Promises returned by native function calls or serve any requested resources.

It's also possible to run a frontend project in a development server and have the `WebBrowserComponent` load it from there, which allows for a faster development cycle. This is also possible to test with the WebViewPluginDemo example by uncommenting the line calling `goToURL` with the local dev server address.

## License

See [LICENSE.md](LICENSE.md) for the license information.
