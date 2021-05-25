# JUCE Accessibility

## What is supported?

Currently JUCE supports VoiceOver on macOS and Narrator on Windows. The JUCE
accessibility API exposes the following to these clients:

  - Title, description, and help text for UI elements
  - Programmatic access to UI elements and text
  - Interaction with UI elements
  - Full UI keyboard navigation
  - Posting notifications to listening clients

## Customising Behaviour

By default any visible and enabled `Component` is accessible to screen reader
clients and exposes some basic information such as title, description, help
text and its position in the hierarchy of UI elements.

The `setTitle()`, `setDescription()` and `setHelpText()` methods can be used
to customise the text that will be read out by accessibility clients when
interacting with UI elements and the `setExplicitFocusOrder()`,
`setFocusContainerType()` and `createFocusTraverser()` methods can be used to
control the parent/child relationships and the order of navigation between UI
elements.

## Custom Components

For further customisation of accessibility behaviours the `AccessibilityHandler`
class provides a unified API to the underlying native accessibility libraries.

This class wraps a component with a given role specified by the
`AccessibilityRole` enum and takes a list of optional actions and interfaces to
provide programmatic access and control over the UI element. Its state is used
to convey further information to accessibility clients via the
`getCurrentState()` method.

To implement the desired behaviours for a custom component, subclass
`AccessibilityHandler` and return an instance of this from the
`Component::createAccessibilityHandler()` method.

## Further Reading

  - [NSAccessibility protocol](https://developer.apple.com/documentation/appkit/nsaccessibility?language=objc)
  - [UI Automation for Win32 applications](https://docs.microsoft.com/en-us/windows/win32/winauto/entry-uiauto-win32)
  - A talk giving an overview of this feature from ADC 2020 can be found on
    YouTube at https://youtu.be/BqrEv4ApH3U

