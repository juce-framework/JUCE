# JUCE breaking changes

# Version 8.0.7

## Change

The default Visual Studio project settings for "Debug Information Format" and
"Force Generation of Debug Symbols" have changed in the Projucer. By default
debug symbols are generated using the /Z7 flag.

**Possible Issues**

PDB file generation may change depending on the combination of "Debug
Information Format" and "Force Generation of Debug Symbols" settings.

**Workaround**

Change the "Debug Information Format" and "Force Generation of Debug Symbols"
settings for each Visual Studio configuration as required.

**Rationale**

The default behaviour of using "Program Database (/Zi)" is incompatible with
some CI workflows and caching mechanisms. Enabling "Force Generation of Debug
Symbols" by default also ensures /Z7 behaves more like /Zi by always generating
a PDB file.


## Change

The signatures of virtual functions ImagePixelData::applyGaussianBlurEffect()
and ImagePixelData::applySingleChannelBoxBlurEffect() have changed.
ImageEffects::applyGaussianBlurEffect() and
ImageEffects::applySingleChannelBoxBlurEffect() have been removed.

**Possible Issues**

User code overriding or calling these functions will fail to compile.

**Workaround**

The blur functions now operate within a specified area of the image. Update
overriding implementations accordingly. Instead of using the ImageEffects
static functions, call the corresponding ImagePixelData member functions
directly.

**Rationale**

The blur functions had a 'temporary storage' parameter which was not
particularly useful in practice, so this has been removed. Moving the
functionality of the ImageEffects static members directly into corresponding
member functions of ImagePixelData simplifies the public API.


# Version 8.0.5

## Change

HeaderItemComponent::getIdealSize no longer applies modifiers to the result
directly. Instead, these changes have been moved to the respective LookAndFeel
methods, enabling better customization.

**Possible Issues**

Code that overrides LookAndFeel::getIdealPopupMenuItemSize and relied on the
previous modifiers applied in HeaderItemComponent::getIdealSize may now behave
differently.

**Workaround**

Review any overrides of LookAndFeel::getIdealPopupMenuItemSize and apply the
necessary adjustments to account for any missing modifiers or changes in
behavior.

**Rationale**

The previous approach did not allow users to customize the applied modifiers
through the LookAndFeel class. Moving this logic to LookAndFeel methods ensures
consistent and flexible customization.


## Change

The behavior of AudioTransportSource::hasStreamFinished has been updated to
correctly return true when the stream has finished.

**Possible Issues**

This change may affect any code that relied on the previous behavior, where the
method never returned true.

**Workaround**

Review and update any code that depends on hasStreamFinished or any registered
ChangeListeners that respond to stream completion.

**Rationale**

The previous behavior, where hasStreamFinished never returned true, was
incorrect. This update ensures the method works as intended.


## Change

AudioProcessor::TrackProperties now uses std::optional.

**Possible Issues**

Code that accessed TrackProperties properties directly will no longer compile.

**Workaround**

Use std::optional::has_value() to check if a property is set. Or Access the
property value safely using std::optional::value() or operator*.

**Rationale**

Previously, it was not possible to distinguish whether a TrackProperty was
explicitly set or if the default value was being used.


## Change

Support for Arm32 in Projucer has been removed for Windows targets.

**Possible Issues**

Projucer projects targeting Arm32 on Windows will no longer be able to select
that option.

**Workaround**

Select Arm64 or Arm64EC instead of Arm32, and port any 32-bit specific code to
64-bit.

**Rationale**

32-bit Arm support has been deprecated in current versions of Windows 11.


# Version 8.0.4

## Change

The Javascript implementation has been moved into a independent juce module.

**Possible Issues**

Any existing use of JavascriptEngine, JSCursor, or JSObject will fail to
compile.

**Workaround**

Add the new juce_javascript module to the project.

**Rationale**

The Javascript implementation increases compilation times while being required
by only a select number of projects.


## Change

The return type for VST3ClientExtensions::getCompatibleClasses() has changed
from a String to an array of 16 bytes.

**Possible Issues**

Any inherited classes overriding this method might fail to compile.

**Workaround**

Either explicitly switch to creating a 16-byte std::array or use
VST3ClientExtensions::toInterfaceId() to convert a string to a 16-byte array.

**Rationale**

As part of adding functionality to support migrating parameter IDs from
compatible plugins it was useful to switch to a safer type for representing
VST3 interface IDs that closer matches the VST3 SDK types.


## Change

The VBlankAttachment class' inheritance from the ComponentPeer::VBlankListener
and ComponentListener classes has been made private.

**Possible Issues**

External code that calls VBlankAttachment::onVBlank or
VBlankAttachment::componentParentHierarchyChanged will fail to compile.

**Workaround**

There is no workaround.

**Rationale**

Making the inheritance public originally was an oversight. The overriden
functions are meant to be called only by the ComponentPeer and Component objects
that the VBlankAttachment instance registers itself with. External code calling
these functions undermines the correct behaviour of the VBlankAttachment class.


## Change

The signature of VBlankListener::onVBlank() was changed to
VBlankListener::onVBlank (double), with the addition of a timestamp parameter
that corresponds to the time at which the next frame will be displayed.

**Possible Issues**

Code that overrides VBlankListener::onVBlank() will fail to compile.

**Workaround**

Add a double parameter to the function overriding VBlankListener::onVBlank().
The behaviour will be unchanged if this new parameter is then ignored.

**Rationale**

A timestamp parameter has been missing from the VBlank callback since its
addition. The new parameter allows all VBlankListeners to synchronise the
content of their draw calls to the same frame timestamp.


# Version 8.0.2

## Change

Font::getStringWidth and Font::getStringWidthFloat have been deprecated.
Font::getGlyphPositions has been removed.

**Possible Issues**

Code that uses these functions will raise warnings at compile time, or fail
to build.

**Workaround**

Use GlyphArrangement::getStringWidth or TextLayout::getStringWidth to find the
width of a string taking font-fallback and shaping into account.

To find individual glyph positions, lay out the string using GlyphArrangement
or TextLayout, then use the positions provided by
GlyphArrangement::PositionedGlyph and/or TextLayout::Glyph.

**Rationale**

The results of the old Font member functions computed their results assuming
that ligatures and other font features would not be used when rendering the
string. The functions would also substitute missing characters with the Font's
notdef/tofu glyph instead of using a fallback font.

Using GlyphArrangement or TextLayout will use a sophisticated text shaping
algorithm to lay out the string, with support for font fallback.


## Change

The constructors of the WebSliderRelay, WebToggleButtonRelay and
WebComboBoxRelay classes were changed and they no longer accept a reference
parameter to a WebBrowserComponent object.

**Possible Issues**

Code that uses these classes will fail to compile.

**Workaround**

Omit the WebBrowserComponent parameter when constructing the relay objects.

**Rationale**

The relay classes use a new underlying mechanism to obtain a pointer to the
WebBrowserComponent object. When calling the
WebBrowserComponent::Options::withOptionsFrom() function with the relay as a
parameter, the corresponding WebBrowserComponent object will notify the relay
about its creation and destruction.

This avoids the anti-pattern where the relay class required a reference to a
yet uninitialised WebBrowserComponent object.


## Change

The coefficients of LadderFilter::Mode::BPF12 have been changed, causing a
slight change in the filter's transfer function.

**Possible Issues**

Code that uses the LadderFilter in BPF12 mode may produce different output
samples.

**Workaround**

There is no workaround. If you need this functionality, please let us know
about your use case. In the meantime, you may be able to copy the old class
into your own project/module and use it that way.

**Rationale**

The LadderFilter implementation follows the paper Valimaki (2006): Oscillator
and Filter Algorithms for Virtual Analog Synthesis. The BPF12 mode coefficients
however contained a typo compared to the paper, making the BPF12 mode incorrect.


# Version 8.0.1

## Change

All member functions of DynamicObject other than clone() and writeAsJSON() have
been made non-virtual.

**Possible Issues**

Classes that override these functions will fail to compile.

**Workaround**

Instead of overriding hasMethod() and invokeMethod(), call setMethod() to
add new member functions.

Instead of overriding getProperty() to return a custom property, add that
property using setProperty().

**Rationale**

Allowing the implementations of these functions to be changed may cause derived
types to accidentally break the invariants of the DynamicObject type.
Specifically, the results of hasMethod() and hasProperty() must be consistent
with the result of getProperties(). Additiionally, calling getProperty() should
return the same var as fetching the property through getProperties(), and
calling invokeMethod() should behave the same way as retrieving and invoking a
NativeFunction via getProperties().

More concretely, the new QuickJS-based Javascript engine requires that all
methods/properties are declared explicitly, which cannot be mapped to the more
open-ended invokeMethod() API taking an arbitrary method name. Making
invokeMethod() non-virtual forces users to add methods with setMethod() instead
of overriding invokeMethod(), which is more compatible with QuickJS.


## Change

The default JSON encoding has changed from ASCII escape sequences to UTF-8.

**Possible Issues**

JSON text exchanged with a non-standard compliant parser expecting ASCII
encoding, may fail to parse UTF-8 encoded JSON files. Reliance on the raw JSON
encoded string literal, for example for file comparison, Base64 encoding, or any
encryption, may result in false negatives for JSON data containing the same data
between versions of JUCE.

Note: JSON files that only ever encoded ASCII text will NOT be effected.

**Workaround**

Use the `JSON::writeToStream()` or `JSON::toString()` functions that take a
`FormatOptions` parameter and call `withEncoding (JSON::Encoding::ascii)` on the
`FormatOptions` object.

**Rationale**

RFC 8259 states

> JSON text exchanged between systems that are not part of a closed ecosystem
MUST be encoded using UTF-8 [RFC3629].
>
> Previous specifications of JSON have not required the use of UTF-8 when
transmitting JSON text.  However, the vast majority of JSON-based software
implementations have chosen to use the UTF-8 encoding, to the extent that it is
the only encoding that achieves interoperability.

For this reason UTF-8 encoding has better interoperability than ASCII escape
sequences.


## Change

The ASCII and Unicode BEL character (U+0007) escape sequence has changed in the
JSON encoder from "\a" to "\u0007".

**Possible Issues**

Reliance on the raw JSON encoded string literal, for example for file comparison,
base-64 encoding, or any encryption, may result in false negatives for JSON data
containing a BEL character between versions of JUCE.

**Workaround**

Use string replace, for example call `replace ("\\u007", "\\a")` on the
resulting JSON string to match older versions of JUCE.

**Rationale**

The JSON specification does not state that the BEL character can be escaped
using "\a". Therefore other JSON parsers incorrectly read this character when
they encounter it.


## Change

The LowLevelGraphicsPostscriptRenderer has been removed.

**Possible Issues**

Code that uses this class will no longer compile.

**Workaround**

There is no workaround. If you need this functionality, please let us know
about your use case. In the meantime, you may be able to copy the old classes
into your own project/module and use them that way.

**Rationale**

We are not aware of any projects using this functionality. This renderer was
not as fully-featured as any of the other renderers, so it's likely that users
would have filed issue reports if they were using this feature.


## Change

Support for the MinGW toolchain has been removed.

**Possible Issues**

MinGW can no longer be used to build JUCE.

**Workaround**

On Windows, use an alternative compiler such as Clang or MSVC.

Cross-compiling for Windows from Linux is not supported, and there is no
workaround for this use case.

**Rationale**

The MinGW provides a poor user experience, with very long build times and
missing features. The high maintenance cost, both in terms of developer time,
and continuous integration bandwidth (both of which could provide more value
elsewhere), means that continued support for MinGW is difficult to justify.


## Change

The GUI Editor has been removed from the Projucer.

**Possible Issues**

The Projucer can no longer be used to visually edit JUCE Components.

**Workaround**

There is no workaround.

**Rationale**

This feature has been deprecated, without receiving bugfixes or maintenance,
for a long time.


## Change

The Visual Studio 2017 exporter has been removed from the Projucer.

**Possible Issues**

It will no longer be possible to generate Visual Studio 2017 projects using the
Projucer.

**Workaround**

Use a different exporter, such as the exporter for Visual Studio 2019 or 2022.

**Rationale**

Since JUCE 8, the minimum build requirement has been Visual Studio 2019. This
minimum requirement allows JUCE to use modern C++ features, along with modern
Windows platform features.


## Change

The Code::Blocks exporter has been removed from the Projucer.

**Possible Issues**

It will no longer be possible to generate Code::Blocks projects using the
Projucer.

**Workaround**

Use a different exporter, such as the Makefile exporter on Linux, or one of the
Visual Studio exporters on Windows.

**Rationale**

The Code::Blocks IDE does not seem to be actively maintained. Other projects
are dropping support, with the Code::Blocks generator deprecated in CMake 3.27.
Additionally, the Code::Blocks exporter did not provide a good user experience,
especially for new users on Windows, as it defaulted to using the MinGW
toolchain. This toolchain tends to be slow to build and link, and is not fully
supported by JUCE, missing support for some audio and video backends, and
plugin formats.


## Change

The tab width when rendering text with the GlyphArrangement and TextLayout
classes now equals the width of a space. Previously it equaled the width of a
tofu character used for missing glyphs.

**Possible Issues**

User interfaces using the GlyphArrangement and TextLayout classes directly to
render text containing tabs will look differently. The TextEditor and
CodeEditorComponent classes have special logic for replacing the tabs prior to
rendering, and consequently, these are not affected.

**Workaround**

Replace the tab characters prior to rendering and substitute them with the
required number of non-breaking spaces.

**Rationale**

Since the Unicode related revamping of JUCE's text rendering classes, tab
characters would raise assertions and would be rendered with the tofu glyph.
This change visually treats tab characters as non-breaking spaces. Since the
JUCE 7 behaviour of using the tofu glyph's width was not a conscious decision,
but rather a side effect of ignoring unresolved glyphs, using a default width
of one space is more reasonable.


# Version 8.0.0

## Change

The virtual functions ResizableWindow::getBorderThickness() and
ResizableWindow::getContentComponentBorder() are now const.

**Possible Issues**

Classes overriding these functions will fail to compile.

**Workaround**

Add 'const' to overriding functions.

**Rationale**

Omitting 'const' from these functions implies that they may change the state of
the ResizableWindow, which would be unexpected behaviour for getter functions.
It also means that the functions cannot be called from const member functions,
which limits their usefulness.


## Change

As part of the Unicode upgrades TextLayout codepaths have been unified across
all platforms. As a consequence the behaviour of TextLayout on Apple platforms
will now be different in two regards:
- With certain fonts, line spacing will now be different.
- The AttributedString option WordWrap::byChar will no longer have an effect,
  just like it didn't have an effect on non-Apple platforms previously. Wrapping
  will now always happen on word boundaries.

Furthermore, the JUCE_USE_DIRECTWRITE compiler flag will no longer have any
effect.

**Possible Issues**

User interfaces using TextLayout and the WordWrap::byChar option will have their
appearance altered on Apple platforms. The line spacing will be different for
certain fonts.

**Workaround**

There is no workaround.

**Rationale**

The new, unified codepath has better support for Unicode text in general. The
font fallback mechanism, which previously was only available using the removed
codepaths is now an integral part of the new approach. By removing the
alternative codepaths, text layout and line spacing has become more consistent
across the platforms.


## Change

As part of the Unicode upgrades the vertical alignment logic of TextLayout has
been altered. Lines containing text written in multiple different fonts will
now have their baselines aligned. Additionally, using the
Justification::verticallyCentred or Justification::bottom flags may now result
in the text being positioned slightly differently.

**Possible Issues**

User interfaces using TextLayout with texts drawn using multiple fonts will now
have their look changed.

**Workaround**

There is no workaround.

**Rationale**

The old implementation had incosistent vertical alignment behaviour. Depending
on what exact fonts the first line of text happened to use, the bottom alignment
would sometimes produce unnecessary padding on the bottom. With certain text and
Font combinations the text would be drawn beyond the bottom boundary even though
there was free space above the text.

The same amount of incorrect vertical offset, that was calculated for bottom
alignment, was also present when using centred, it just wasn't as apparent.

Not having the baselines aligned between different fonts resulted in generally
displeasing visuals.


## Change

The virtual functions LowLevelGraphicsContext::drawGlyph() and drawTextLayout()
have been removed.

**Possible Issues**

Classes overriding these functions will fail to compile.

**Workaround**

Replace drawGlyph() with drawGlyphs(), which draws several glyphs at once.
Remove implementations of drawTextLayout().

**Rationale**

On Windows and macOS, drawing several glyphs at once is faster than drawing
glyphs one-at-a-time. The new API is more general, and allows for more
performant text rendering.


## Change

JUCE widgets now query the LookAndFeel to determine the TypefaceMetricsKind to
use. By default, the LookAndFeel will specify the "portable" metrics kind,
which may change the size of text in JUCE widgets, depending on the font and
platform.

**Possible Issues**

Using "portable" metrics may cause text to render at a different scale when
compared to the old "legacy" metrics.

**Workaround**

If you want to restore the old metrics, e.g. to maintain the same text scaling
in an existing app, you can override LookAndFeel::getDefaultMetricsKind() on
each LookAndFeel in your application, to return the "legacy" metrics kind.

**Rationale**

Using portable font metrics streamlines the development experience when working
on applications that must run on multiple platforms. Using portable metrics by
default means that new projects will benefit from this improved cross-platform
behaviour from the outset.


## Change

Signatures of several Typeface member functions have been updated to accept a
new TypefaceMetricsKind argument. The getAscent(), getDescent(), and
getHeightToPointsFactor() members have been replaced by getMetrics(), which
returns the same metrics information all at once.

Font instances now store a metrics kind internally. Calls to Font::getAscent()
and other functions that query font metrics will always use the Font's stored
metrics kind. Calls to Font::operator== will take the metrics kinds into
account, so two fonts that differ only in their stored metrics kind will
be considered non-equal.

**Possible Issues**

Code that calls any of the affected Typeface functions will fail to compile.
Code that compares Font instances may behave differently if the compared font
instances use mismatched metrics kinds.

**Workaround**

Specify the kind of metrics you require when calling Typeface member functions.
Call getMetrics() instead of the old individual getters for metrics. Review
calls to Font::operator==, especially where comparing against a
default-constructed Font.

**Rationale**

Until now, the same font data could produce different results from
Typeface::getAscent() et al. depending on the platform. The updated interfaces
allow the user to choose between the old-style non-portable metrics (to avoid
layout changes in existing projects), and portable metrics (more suitable for
new or cross-platform projects).
Most users will fetch metrics from Font objects rather than from the Typeface.
Font will continue to return non-portable metrics when constructed using the
old (deprecated) constructors. Portable metrics can be enabled by switching to
the new Font constructor that takes a FontOptions argument. See the
documentation for TypefaceMetricsKind for more details.


## Change

Typeface::getOutlineForGlyph now returns void instead of bool.

**Possible Issues**

Code that checks the result of this function will fail to compile.

**Workaround**

Omit any checks against the result of this function.

**Rationale**

This function can no longer fail. It may still output an empty path if the
requested glyph isn't present in the typeface.


## Change

CustomTypeface has been removed.

**Possible Issues**

Code that interacts with CustomTypeface will fail to compile.

**Workaround**

There is currently no workaround. If you were using CustomTypeface to
implement typeface fallback, there is a new API,
Font::findSuitableFontForText, that you can use to locate fonts capable
of rendering given strings.

**Rationale**

The CustomTypeface class is difficult/impossible to support with the new
HarfBuzz Typeface implementation. New support for automatic font fallback
will be introduced in JUCE 8, and this will obviate much of the need for
CustomTypeface.


## Change

The Android implementations of Typeface::getStringWidth(), getGlyphPositions(),
and getEdgeTableForGlyph() have been updated to return correctly-normalised
results. The effect of this change is to change (in practice, slightly reduce)
the size at which many fonts will render on Android.

**Possible Issues**

The scale of some text on Android may change.

**Workaround**

For font sizes specified in 'JUCE units' by passing a value to the Font
constructor or to Font::setHeight, instead pass the same size to
Font::withPointHeight and use the returned Font object.

**Rationale**

The behaviour of the Typeface member functions did not match the documented
behaviour, or the behaviour on other platforms. This could make it difficult to
create interfaces that rendered as expected on multiple platforms.

The upcoming unicode support work will unify much of the font-handling and
text-shaping machinery in JUCE. Ensuring that all platforms have consistent
behaviour before and after the unicode upgrade will make it easier to implement
and verify those changes.


## Change

The JavascriptEngine::callFunctionObject() function has been removed.

**Possible Issues**

Projects that used the removed function will fail to compile.

**Workaround**

Use the JSObjectCursor::invokeMethod() function to call functions beyond the
root scope.

**Rationale**

The JavascriptEngine's underlying implementation has been changed, and the
DynamicObject type is no longer used for the internal implementation of the
engine. The JSObjectCursor class provides a way to navigate the Javascript
object graph without depending on the type of the engine's internal
implementation.


## Change

The JavascriptEngine::getRootObjectProperties() function returns its result by
value instead of const reference.

**Possible Issues**

Projects that captured the returned value by reference and depended on it being
valid for more than the current function's scope may stop working correctly.

**Workaround**

If the return value is used beyond the calling function's scope it must be
stored in a value.

**Rationale**

The JavascriptEngine's underlying implementation has been changed, and the
NamedValueSet type is no longer used in its internal representation. Hence a new
NamedValueSet object is created during the getRootObjectProperties() function
call.


## Change

JavascriptEngine::evaluate() will now return a void variant if the passed in
code successfully evaluates to void, and only return an undefined variant if
an error occurred during evaluation. The previous implementation would return
var::undefined() in both cases.

**Possible Issues**

Projects that depended on the returned value of JavascriptEngine::evaluate() to
be undefined even during successful evaluation may fail to work correctly.

**Workaround**

Code paths that depend on an undefined variant to be returned should be checked
if they aren't used exclusively to determine evaluation failure. In failed
cases the JavascriptEngine::evaluate() function will continue to return
var::undefined().

**Rationale**

When a Javascript expression successfully evaluates to void, and when it fails
evaluation due to timeout or syntax errors are distinctly different situations
and this should be reflected on the value returned.


## Change

The old JavascriptEngine internals have been entirely replaced by a new
implementation wrapping the QuickJS engine.

**Possible Issues**

Code that previously successfully evaluated using JavascriptEngine::evaluate()
or JavascriptEngine::execute(), could now fail due to the rules applied by the
new, much more standards compliant engine. One example is object literals
e.g. { a: 'foo', b: 42, c: {} }. When evaluated this way the new engine will
assume that this is a code block and fail.

**Workaround**

When calling JavascriptEngine::evaluate() or JavascriptEngine::execute() the
code may have to be updated to ensure that it's correct according to the
Javascript language specification and in the context of that evaluation. Object
literals standing on their own for example should be wrapped in parentheses
e.g. ({ a: 'foo', b: 42, c: {} }).

**Rationale**

The new implementation uses a fully featured, performant, standards compliant
Javascript engine, which is a significant upgrade.


## Change

The `WebBrowserComponent::pageAboutToLoad()` function on Android now only
receives callbacks for entire page navigation events, as opposed to every
resource fetch operation. Returning `false` from the function now prevents
this operation from taking any effect, as opposed to producing potentially
visible error messages.

**Possible Issues**

Code that previously depended on the ability to allow or fail resource
requests on Android may fail to work correctly.

**Workaround**

Navigating to webpages can still be prevented by returning `false` from this
function, similarly to other platforms.

Resource requests sent to the domain returned by
`WebBrowserComponent::getResourceProviderRoot()` can be served or rejected by
using the `WebBrowserComponent::ResourceProvider` feature.

Resource requests sent to other domains can not be controlled on Android
anymore.

**Rationale**

Prior to this change there was no way to reject a page load operation without
any visible effect, like there was on the other platforms. The fine grained per
resource control was not possible on other platforms. This change makes the
Android implementation more consistent with the other platforms.


## Change

The minimum supported compilers and deployment targets have been updated, with
the new minimums listed in the top level [README](README.md).

MinGW is no longer supported.

**Possible Issues**

You may no longer be able to build JUCE projects or continue targeting older
platforms.

**Workaround**

If you cannot build your project, update your build machine to a more modern
operating system and compiler.

There is no workaround to target platforms that predate the new minimum
deployment targets.

**Rationale**

New features of JUCE require both more modern compilers and deployment targets.

The amount of investment MinGW support requires is unsustainable.


## Change

The [JUCE End User Licence Agreement](https://juce.com/legal/juce-8-licence/)
has been updated and all JUCE modules are now dual-licensed under the AGPLv3 and
the JUCE licence. Previously the juce_audio_basics, juce_audio_devices,
juce_core and juce_events modules were licensed under the ISC licence.

Please read the End User Licence Agreement for full details.

**Possible Issues**

There may be new restrictions on how you can use JUCE.

**Workaround**

N/A

**Rationale**

The new JUCE End User Licence Agreement is much easier to understand, and has a
much more generous personal tier. The move from ISC to AGPLv3/JUCE simplifies
the licensing situation and encourages the creation of more open source software
without impacting personal use of the JUCE framework.


# Version 7.0.12

## Change

The function AudioChannelSet::create9point0point4, along with variants for
9.1.4, 9.0.6, and 9.1.6, used to correspond to VST3 layouts k90_4, k91_4,
k90_6, and k91_6 respectively. These functions now correspond to k90_4_W,
k91_4_W, k90_6_W, and k91_6_W respectively.

**Possible Issues**

VST3 plugins that used these AudioChannelSet layouts to specify initial bus
layouts, or to validate layouts in isBusesLayoutSupported, will now behave
differently.

For example, if the host wants to check whether the k90_4 layout is supported,
previously isBusesLayoutSupported() would have received the layout created by
create9point0point4(), but will now receive the layout created by
create9point0point4ITU().

**Workaround**

If you already have special-case handling for specific surround layouts,
e.g. to enable or disable them in isBusesLayoutSupported(), you may need to
add cases to handle the new AudioChannelSet::create*ITU() layout variants.

**Rationale**

Previously, the VST3 SDK only contained ITU higher-order surround layouts, but
the higher-order layouts specified in JUCE used Atmos speaker positions rather
than ITU speaker positions. This meant that JUCE had to remap speaker layouts
between Atmos/ITU formats when communicating with VST3 plugins. This was
confusing, as it required that the meaning of some channels was changed during
the conversion.

In newer versions of the VST3 SDK, new "wide" left and right speaker
definitions are available, allowing both ITU and Atmos surround layouts to be
represented. The change in JUCE surfaces this distinction to the user, allowing
them to determine e.g. whether the host has requested an ITU or an Atmos
layout, and to handle these cases separately if necessary.


# Version 7.0.10

## Change

The signatures of some member functions of ci::Device have been changed:
- sendPropertyGetInquiry
- sendPropertySetInquiry

The signature of ci::PropertyHost::sendSubscriptionUpdate has also changed.

The following member functions of ci::Device have been replaced with new
alternatives:
- sendPropertySubscriptionStart
- sendPropertySubscriptionEnd
- getOngoingSubscriptionsForMuid
- countOngoingPropertyTransactions

The enum field PropertyExchangeResult::Error::invalidPayload has been removed.

**Possible Issues**

Code that uses any of these symbols will fail to compile until it is updated.

**Workaround**

Device::sendPropertyGetInquiry, Device::sendPropertySetInquiry, and
PropertyHost::sendSubscriptionUpdate all now return an optional RequestKey
instead of an ErasedScopeGuard. Requests started via any of these functions may
be cancelled by the request's RequestKey to the new function
Device::abortPropertyRequest. The returned RequestKey may be null, indicating a
failure to send the request.

countOngoingPropertyTransactions has been replaced by getOngoingRequests,
which returns the RequestKeys of all ongoing requests. To find the number of
transactions, use the size of the returned container.

sendPropertySubscriptionStart has been replaced by beginSubscription.
sendPropertySubscriptionEnd has been replaced by endSubscription.
The new functions no longer take callbacks. Instead, to receive notifications
when a subscription starts or ends, override
DeviceListener::propertySubscriptionChanged.

getOngoingSubscriptionsForMuid is replaced by multiple functions.
getOngoingSubscriptions returns SubscriptionKeys for all of the subscriptions
currently in progress, which may be filtered based on SubscriptionKey::getMuid.
The subscribeId assigned to a particular SubscriptionKey can be found using
getSubscribeIdForKey, and the subscribed resource can be found using
getResourceForKey.

It's possible that the initial call to beginSubscription may not be able to
start the subscription, e.g. if the remote device is busy and request a retry.
In this case, the request is cached. If you use subscriptions, then you
should call sendPendingMessages periodically to flush any messages that may
need to be retried.

There is no need to check for the invalidPayload error when processing
property exchange results.

**Rationale**

Keeping track of subscriptions is quite involved, as the initial request to
begin a subscription might not be accepted straight away. The device may not
initially have enough vacant slots to send the request, or responder might
request a retry if it is too busy to process the request. The ci::Device now
caches requests when necessary, allowing them to be retried in the future.
This functionality couldn't be implemented without modifying the old interface.

Replacing ErasedScopeGuards with Keys makes lifetime handling a bit easier.
It's no longer necessary to store or manually release scope guards for requests
that don't need to be cancelled. The new Key types are also a bit more
typesafe, and allow for simple queries of the transaction that created the key.


## Change

The ListenerList::Iterator class has been removed.

**Possible Issues**

Any code directly referencing the ListenerList::Iterator will fail to compile.

**Workaround**

In most cases there should be a public member function that does the required
job, for example, call, add, remove, or clear. In other cases you can access the
raw array of listeners to iterate through them by calling getListeners().

**Rationale**

Iterating through the listeners using the ListenerList::Iterator could in a
number of cases lead to surprising results and undefined behavior.


## Change

The background colour of the Toolbar::CustomisationDialog has been changed from
white to a new, customisable value, that matches Toolbar::backgroundColourId by
default.

**Possible Issues**

User interfaces that use Toolbar::CustomisationDialog will render differently.

**Workaround**

You can customise the new colour using LookAndFeel::setColour() using
Toolbar::customisationDialogBackgroundColourId.

**Rationale**

Previously there was no way to customise the dialog's background colour and the
fixed white colour was inappropriate for most user interfaces.


## Change

ProfileHost::enableProfile and ProfileHost::disableProfile have been combined
into a single function, ProfileHost::setProfileEnablement.

**Possible Issues**

Code that calls this function will fail to compile until it is updated.

**Workaround**

To enable a profile, call setProfileEnablement with a positive number of
channels. To disable a profile, call setProfileEnablement with zero channels.

**Rationale**

The new API is simpler, more compact, and more consistent, as it now mirrors
the signature of Device::sendProfileEnablement.


## Change

OpenGLContext::getRenderingScale() has been changed to include the effects of
AffineTransforms on all platforms.

**Possible Issues**

Applications that use OpenGLContext::getRenderingScale() and also have scaling
transformations that affect the context component's size may render incorrectly.

**Workaround**

Adjust rendering code by dividing the reported scale with the user specified
transformation scale, if necessary.

**Rationale**

The previous implementation resulted in inconsistent behaviour between Windows
and the other platforms. The main intended use-case for getRenderingScale() is
to help determine the number of physical pixels covered by the context
component. Since plugin windows will often use AffineTransforms to set up the
correct rendering scale, it makes sense to include these in the result of
getRenderingScale().


## Change

Components that have setMouseClickGrabsKeyboardFocus() set to false will not
accept or propagate keyboard focus to parent components due to a mouse click
event. This is now true even if the mouse click event happens in a child
component with setMouseClickGrabsKeyboardFocus (true) and
setWantsKeyboardFocus (false).

**Possible Issues**

Components that rely on child components propagating keyboard focus from a
mouse click, when those child components have setMouseClickGrabsKeyboardFocus()
set to false, will no longer grab keyboard focus.

**Workaround**

Add a MouseListener to the component receiving the click and override the
mouseDown() method in the listener. In the mouseDown() method call
Component::grabKeyboardFocus() for the component that should be focused.

**Rationale**

The intent of setMouseClickGrabsKeyboardFocus (false) is to reject focus changes
coming from mouse clicks even if the component is otherwise capable of receiving
keyboard focus.

The previous behaviour could result in surprising focus changes when a child
component was clicked. This manifested in the focus seemingly disappearing when
a PopupMenu item added to a component was clicked.


## Change

The NodeID argument to AudioProcessorGraph::addNode() has been changed to take
a std::optional<NodeID>.

**Possible Issues**

The behavior of any code calling AudioProcessorGraph::addNode(), that explicitly
passes a default constructed NodeID or a NodeID constructed with a value of 0,
will change. Previously these values would have been treated as a null value
resulting in the actual NodeID being automatically determined. These will now
be treated as requests for an explicit value.

**Workaround**

Either remove the explicit NodeID argument and rely on the default argument or
pass a std::nullopt instead.

**Rationale**

The previous version prevented users from specifying a NodeID of 0 and resulted
in unexpected behavior.


## Change

The signature of DynamicObject::writeAsJSON() has been changed to accept a
more extensible JSON::FormatOptions argument.

**Possible Issues**

Code that overrides or calls this function will fail to compile.

**Workaround**

Update the signatures of overriding functions. Use FormatOptions::getIndentLevel()
and FormatOptions::getMaxDecimalPlaces() as necessary. To find whether the output
should be multi-line, compare the result of FormatOptions::getSpacing() with
JSON::Spacing::multiLine.

Callers of the function can construct the new argument type using the old
arguments accordingly

```
JSON::FormatOptions{}.withIndentLevel (indentLevel)
                     .withSpacing (allOnOneLine ? JSON::Spacing::singleLine
                                                : JSON::Spacing::multiLine)
                     .withMaxDecimalPlaces (maximumDecimalPlaces);
```

**Rationale**

The previous signature made it impossible to add new formatting options. Now,
if we need to add further options in the future, these can be added to the
FormatOptions type, which will not be a breaking change.


# Version 7.0.9

## Change

CachedValue::operator==() will now emit floating point comparison warnings if
they are enabled for the project.

**Possible Issues**

Code using this function to compare floating point values may fail to compile
due to the warnings.

**Workaround**

Rather than using CachedValue::operator==() for floating point types, use the
exactlyEqual() or approximatelyEqual() functions in combination with
CachedValue::get().

**Rationale**

The JUCE Framework now offers the free-standing exactlyEqual() and
approximatelyEqual() functions to clearly express the desired semantics when
comparing floating point values. These functions are intended to eliminate
the ambiguity in code-bases regarding these types. However, when such a value
is wrapped in a CachedValue the corresponding warning was suppressed until now,
making such efforts incomplete.


# Version 7.0.8

## Change

DynamicObject::clone now returns unique_ptr<DynamicObject> instead of
ReferenceCountedObjectPtr<DynamicObject>.

**Possible Issues**

Overrides of this function using the old signature will fail to compile.
The result of this function may need to be manually converted to a
ReferenceCountedObjectPtr.

**Workaround**

Update overrides to use the new signature.
If necessary, manually construct a ReferenceCountedObjectPtr at call sites.

**Rationale**

It's easy to safely upgrade a unique_ptr to a shared/refcounted pointer.
However, it's not so easy to convert safely in the opposite direction.
Generally, returning unique_ptrs rather than refcounted pointers leads to more
flexible APIs.


# Version 7.0.7

## Change

The minimum supported CMake version is now 3.22.

**Possible Issues**

It will no longer be possible to configure JUCE projects with CMake versions
between 3.15 and 3.21 inclusive.

**Workaround**

No workaround is available. Newer versions of CMake can be obtained from the
official download page, or through system package managers.

**Rationale**

Moving to CMake 3.22 improves consistency with the Projucer's Android exporter,
which already requires CMake 3.22. It also allows us to make use of the
XCODE_EMBED_APP_EXTENSIONS property (introduced in CMake 3.21), fixing an
issue when archiving AUv3 plugins.


# Version 7.0.6

## Change

Thread::wait and WaitableEvent::wait now take a double rather than an int to
indicate the number of milliseconds to wait.

**Possible Issues**

Calls to either wait function may trigger warnings.

**Workaround**

Explicitly cast the value to double.

**Rationale**

Changing to double allows sub-millisecond waits which was important for
supporting changes to the HighResolutionTimer.


## Change

RealtimeOptions member workDurationMs was replaced by three optional member
variables in RealtimeOptions, and all RealtimeOptions member variables were
marked private.

**Possible Issues**

Trying to construct a RealtimeOptions object with one or two values, or access
any of its member variables, will no longer compile.

**Workaround**

Use the withMember functions to construct the object, and the getter functions
to access the member variable values.

**Rationale**

The new approach improves the flexibility for users to specify realtime thread
options on macOS/iOS and improves the flexibility for the API to evolve without
introducing further breaking changes.


## Change

JUCE module compilation files with a platform suffix are now checked case
insensitively for CMake builds.

**Possible Issues**

If a JUCE module compilation file ends in a specific platform suffix but does
not match the case for the string previously checked by the CMake
implementation, it may have compiled for all platforms. Now, it will only
compile for the platform specified by the suffix.

**Workaround**

In most cases this was probably a bug, in other cases rename the file to remove
the platform suffix.

**Rationale**

This change improves consistency between the Projucer and CMake integrations.


## Change

An undocumented feature that allowed JUCE module compilation files to compile
for a specific platform or subset of platforms by declaring the platform name
followed by an underscore, was removed.

**Possible Issues**

If a JUCE module compilation file contains a matching platform suffix followed
by an underscore and is loaded by the Projucer it will no longer compile for
just that platform.

**Workaround**

Use the suffix of the name only. If the undocumented feature was used to select
multiple platforms, make multiple separate files for each of the required
platforms.

**Rationale**

This change improves consistency between the Projucer and CMake integrations.
Given the functionality was undocumented, the ease of a workaround, and the
added complexity required for CMake support, the functionality was removed.


## Change

Unique device IDs on iOS now use the OS provided 'identifierForVendor'.
OnlineUnlockStatus has been updated to handle the iOS edge-case where a device
ID query might return an empty String.

**Possible Issues**

The License checks using InAppPurchases, getLocalMachineIDs(), and
getUniqueDeviceID() may return an empty String if iOS 'is not ready'. This can
occur for example if the device has restarted but has not yet been unlocked.

**Workaround**

InAppPurchase has been updated to handle this and propagate the error
accordingly. The relevant methods have been updated to return a Result object
that can be queried for additional information on failure.

**Rationale**

Apple have introduced restrictions on device identification rendering our
previous methods unsuitable.


## Change

AudioProcessor::getAAXPluginIDForMainBusConfig() has been deprecated.

**Possible Issues**

Any AudioProcessor overriding this method will fail to compile.

**Workaround**

- Create an object which inherits from AAXClientExtensions.
- In the object override and implement getPluginIDForMainBusConfig().
- In the AudioProcessor override getAAXClientExtensions() and return a pointer
  to the object.

**Rationale**

Additional AAX specific functionality was required in the audio processor.
Rather than continuing to grow and expand the AudioProcessor class with format
specific functionality, separating this concern into a new class allows for
greater expansion for those that need it without burdening those that don't.
Moving this function into this class improves consistency both with the new
functionality and with similar functionality for the VST2 and VST3 formats.


## Change

Unique device IDs on Windows have been updated to use a more reliable SMBIOS
parser. The SystemStats::getUniqueDeviceID function now returns new IDs using
this improved parser. Additionally, a new function,
SystemStats::getMachineIdentifiers, has been introduced to aggregate all ID
sources. It is recommended to use this new function to verify any IDs.

**Possible Issues**

The SystemStats::getUniqueDeviceID function will return a different ID for the
same machine due to the updated parser.

**Workaround**

For code that previously relied on SystemStats::getUniqueDeviceID, it is advised
to switch to using SystemStats::getMachineIdentifiers() instead.

**Rationale**

This update ensures the generation of more stable and reliable unique device
IDs, while also maintaining backward compatibility with the previous ID
generation methods.


## Change

The Grid layout algorithm has been slightly altered to provide more consistent
behaviour. The new approach guarantees that dimensions specified using the
absolute Px quantity will always be correctly rounded when applied to the
integer dimensions of Components.

**Possible Issues**

Components laid out using Grid can observe a size or position change of +/- 1px
along each dimension compared with the result of the previous algorithm.

**Workaround**

If the Grid based graphical layout is sensitive to changes of +/- 1px, then the
UI layout code may have to be adjusted to the new algorithm.

**Rationale**

The old Grid layout algorithm could exhibit surprising and difficult to control
single pixel artifacts, where an item with a specified absolute size of
e.g. 100px could end up with a layout size of 101px. The new approach
guarantees that such items will have a layout size exactly as specified, and
this new behaviour is also in line with CSS behaviour in browsers. The new
approach makes necessary corrections easier as adding 1px to the size of an
item with absolute dimensions is guaranteed to translate into an observable 1px
increase in the layout size.


## Change

The k91_4 and k90_4 VST3 layouts are now mapped to the canonical JUCE 9.1.4 and
9.0.4 AudioChannelSets. This has a different ChannelType layout than the
AudioChannelSet previously used with such VST3 SpeakerArrangements.

**Possible Issues**

VST3 plugins that were prepared to work with the k91_4 and k90_4
SpeakerArrangements may now have incorrect channel mapping. The channels
previously accessible through ChannelType::left and right are now accessible
through ChannelType::wideLeft and wideRight, and channels previously accessible
through ChannelType::leftCentre and rightCentre are now accessible through
ChannelType::left and right.

**Workaround**

Code that accesses the channels that correspond to the VST3 Speakers kSpeakerL,
kSpeakerR, kSpeakerLc and kSpeakerRc needs to be updated. These channels are now
accessible respectively through ChannelTypes wideLeft, wideRight, left and
right. Previously they were accessible respectively through left, right,
leftCentre and rightCentre.

**Rationale**

This change allows developers to handle the 9.1.4 and 9.0.4 surround layouts
with one codepath across all plugin formats. Previously the
AudioChannelSet::create9point1point4() and create9point0point4() layouts would
only be used in CoreAudio and AAX, but a different AudioChannelSet would be used
in VST3 even though they were functionally equivalent.


## Change

The signatures of the ContentSharer member functions have been updated. The
ContentSharer class itself is no longer a singleton.

**Possible Issues**

Projects that use the old signatures will not build until they are updated.

**Workaround**

Instead of calling content sharer functions through a singleton instance, e.g.
    ContentSharer::getInstance()->shareText (...);
call the static member functions directly:
    ScopedMessageBox messageBox = ContentSharer::shareTextScoped (...);
The new functions return a ScopedMessageBox instance. On iOS, the content
sharer will only remain open for as long as the ScopedMessageBox remains alive.
On Android, this functionality will be added as/when the native APIs allow.

**Rationale**

The new signatures are safer and easier to use. The ScopedMessageBox also
allows content sharers to be dismissed programmatically, which wasn't
previously possible.


## Change

The minimum supported AAX library version has been bumped to 2.4.0 and the
library is now built automatically while building an AAX plugin. The
JucePlugin_AAXLibs_path preprocessor definition is no longer defined in AAX
plugin builds.

**Possible Issues**

Projects that use the JucePlugin_AAXLibs_path definition may no longer build
correctly. Projects that reference an AAX library version earlier than 2.4.0
will fail to build.

**Workaround**

You must download an AAX library distribution with a version of at least 2.4.0.
Use the definition JucePlugin_Build_AAX to check whether the AAX format is
enabled at build time.

**Rationale**

The JUCE framework now requires features only present in version 2.4.0 of the
AAX library. The build change removes steps from the build process, and ensures
that the same compiler flags are used across the entire project.


## Change

The implementation of ColourGradient::createLookupTable has been updated to use
non-premultiplied colours.

**Possible Issues**

Programs that draw transparent gradients using the OpenGL or software
renderers, or that use lookup tables generated from transparent gradients for
other purposes, may now produce different results.

**Workaround**

For gradients to render the same as they did previously, transparent colour
stops should be un-premultiplied. For colours with an alpha component of 0, it
may be necessary to specify appropriate RGB components.

**Rationale**

Previously, transparent gradients rendered using CoreGraphics looked different
to the same gradients drawn using OpenGL or the software renderer. This change
updates the OpenGL and software renderers, so that they produce the same
results as CoreGraphics.


## Change

Projucer-generated MSVC projects now build VST3s as bundles, rather than as
single DLL files.

**Possible Issues**

Build workflows that expect the VST3 to be a single DLL may break.

**Workaround**

Any post-build scripts that expect to copy or move the built VST3 should be
updated so that the entire bundle directory is copied/moved. The DLL itself
can still be located and extracted from within the generated bundle if
necessary.

**Rationale**

Distributing VST3s as single files was deprecated in VST3 v3.6.10. JUCE's CMake
scripts already produce VST3s as bundles, so this change increases consistency
between the two build systems.


# Version 7.0.3

## Change

The default macOS and iOS deployment targets set by the Projucer have been
increased to macOS 10.13 and iOS 11 respectively.

**Possible Issues**

Projects using the Projucer's default minimum deployment target will have their
minimum deployment target increased.

**Workaround**

If you need a lower minimum deployment target then you must set this in the
Projucer's Xcode build configuration settings.

**Rationale**

Xcode 14 no longer supports deployment targets lower than macOS 10.13 and iOS
11.


## Change

The ARA SDK expected by JUCE has been updated to version 2.2.0.

**Possible Issues**

Builds using earlier versions of the ARA SDK will fail to compile.

**Workaround**

The ARA SDK configured in JUCE must be updated to version 2.2.0.

**Rationale**

# Version 2.2.0 is the latest official release of the ARA SDK.


## Change

The Thread::startThread (int) and Thread::setPriority (int) methods have been
removed. A new Thread priority API has been introduced.

**Possible Issues**

Code will fail to compile.

**Workaround**

Rather than using an integer thread priority you must instead use a value from
the Thread::Priority enum. Thread::setPriority and Thread::getPriority should
only be called from the target thread. To start a Thread with a realtime
performance profile you must call startRealtimeThread.

**Rationale**

Operating systems are moving away from a specific thread priority and towards
more granular control over which types of cores can be used and things like
power throttling options. In particular, it is no longer possible to map a 0-10
integer to a meaningful performance range on macOS ARM using the pthread
interface. Using a more modern interface grants us access to more runtime
options, but also changes how we can work with threads. The two most
significant changes are that we cannot mix operations using the new and old
interfaces, and that changing a priority using the new interface can only be
done on the currently running thread.


## Change

The constructor of WebBrowserComponent now requires passing in an instance of
a new Options class instead of a single option boolean. The
WindowsWebView2WebBrowserComponent class was removed.

**Possible Issues**

Code using the WebBrowserComponent's boolean parameter to indicate if a
webpage should be unloaded when the component is hidden, will now fail to
compile. Additionally, any code using the WindowsWebView2WebBrowserComponent
class will fail to compile. Code relying on the default value of the
WebBrowserComponent's constructor are not affected.

**Workaround**

Instead of passing in a single boolean to the WebBrowserComponent's
constructor you should now set this option via tha
WebBrowserComponent::Options::withKeepPageLoadedWhenBrowserIsHidden method.

If you were previously using WindowsWebView2WebBrowserComponent to indicate to
JUCE that you prefer JUCE to use Windows' Webview2 browser backend, you now do
this by setting the WebBrowserComponent::Options::withBackend method. The
WebView2Preferences can now be modified with the methods in
WebBrowserComponent::Options::WinWebView2.

**Rationale**

The old API made adding further options to the WebBrowserComponent cumbersome
especially as the WindowsWebView2WebBrowserComponent already had a parameter
very similar to the above Options class, whereas the base class did not use
such a parameter. Furthermore, using an option to specify the preferred
browser backend is more intuitive then requiring the user to derive from a
special class, especially if additional browser backends are added in the
future.


## Change

The function AudioIODeviceCallback::audioDeviceIOCallback() was removed.

**Possible Issues**

Code overriding audioDeviceIOCallback() will fail to compile.

**Workaround**

Affected classes should override the audioDeviceIOCallbackWithContext() function
instead.

**Rationale**

The audioDeviceIOCallbackWithContext() function fulfills the same role as
audioDeviceIOCallback(), it just has an extra parameter. Hence the
audioDeviceIOCallback() function was superfluous.


## Change

The type representing multi-channel audio data has been changed from T** to
T* const*. Affected classes are AudioIODeviceCallback, AudioBuffer and
AudioFormatReader.

**Possible Issues**

Code overriding the affected AudioIODeviceCallback and AudioFormatReader
functions will fail to compile. Code that interacts with the return value of
AudioBuffer::getArrayOfReadPointers() and AudioBuffer::getArrayOfWritePointers()
may fail to compile.

**Workaround**

Functions overriding the affected AudioIODeviceCallback and AudioFormatReader
members will need to be changed to confirm to the new signature. Type
declarations related to getArrayOfReadPointers() and getArrayOfWritePointers()
of AudioBuffer may have to be adjusted.

**Rationale**

While the previous signature permitted it, changing the channel pointers by the
previously used types was already being considered illegal. The earlier type
however prevented passing T** values to parameters with type const T**. In some
places this necessitated the usage of const_cast. The new signature can bind to
T** values and the awkward casting can be avoided.


## Change

The minimum supported C++ standard is now C++17 and the oldest supported
compilers on Linux are now GCC 7.0 and Clang 6.0.

**Possible Issues**

Older compilers will no longer be able to compile JUCE.

**Workaround**

No workaround is available.

**Rationale**

This compiler upgrade will allow the use of C++17 within the framework.


## Change

Resource forks are no longer generated for Audio Unit plug-ins.

**Possible Issues**

New builds of JUCE Audio Units may no longer load in old hosts that use the
Component Manager to discover plug-ins.

**Workaround**

No workaround is available.

**Rationale**

The Component Manager is deprecated in macOS 10.8 and later, so the majority of
hosts have now implemented support for the new plist-based discovery mechanism.
The new AudioUnitSDK (https://github.com/apple/AudioUnitSDK) provided by Apple
to replace the old Core Audio Utility Classes no longer includes the files
required to generate resource forks.


## Change

Previously, the AudioProcessorGraph would call processBlockBypassed on any
processor for which setBypassed had previously been called. Now, the
AudioProcessorGraph will now only call processBlockBypassed if those processors
do not have dedicated bypass parameters.

**Possible Issues**

Processors with non-functional bypass parameters may not bypass in the same way
as before.

**Workaround**

For each AudioProcessor owned by a Graph, ensure that either: the processor has
a working bypass parameter that correctly affects the output of processBlock();
or, the processor has no bypass parameter, in which case processBlockBypassed()
will be called as before.

**Rationale**

The documentation for AudioProcessor::getBypassParameter() states that if this
function returns non-null, then processBlockBypassed() should never be called,
but the AudioProcessorGraph was breaking this rule. Calling
processBlockBypassed() on AudioProcessors with bypass parameters is likely to
result in incorrect or unexpected output if this function is not overridden.
The new behaviour obeys the contract set out in the AudioProcessor
documentation.


# Version 7.0.2

## Change

The Matrix3D (Vector3D<Type> vector) constructor has been replaced with an
explicit static Matrix3D fromTranslation (Vector3D<Type> vector) function, and a
bug in the behaviour of the multipication operator that reversed the order of
operations has been addressed.

**Possible Issues**

Code using the old constructor will not compile. Code that relied upon the order
of multiplication operations will return different results.

**Workaround**

Code that was using the old constructor must use the new static function. Code
that relied on the order of multiplication operations will need to have the
order of the arguments reversed. With the old code A * B was returning BA rather
than AB.

**Rationale**

Previously a matrix multipled by a vector would return a matrix, rather than a
vector, as the multiplied-by vector would be automatically converted into a
matrix during the operation. Removing the converting constructor makes
everything much more explicit and there is no confusion about dimensionality.
The current multiplication routine also included a bug where A * B resulted in
BA rather than AB, which needed to be addressed.


# Version 7.0.0

## Change

AudioProcessor::getHostTimeNs() and AudioProcessor::setHostTimeNanos() have
been removed.

**Possible Issues**

Code that used these functions will no longer compile.

**Workaround**

Set and get the system time corresponding to the current audio callback using
the new functions AudioPlayHead::PositionInfo::getHostTimeNs() and
AudioPlayHead::PositionInfo::setHostTimeNs().

**Rationale**

This change consolidates callback-related timing information into the
PositionInfo type, improving the consistency of the AudioProcessor and
AudioPlayHead APIs.


## Change

AudioPlayHead::getCurrentPosition() has been deprecated and replaced with
AudioPlayHead::getPosition().

**Possible Issues**

Hosts that implemented custom playhead types may no longer compile. Plugins
that used host-provided timing information may trigger deprecation warnings
when building.

**Workaround**

Classes that derive from AudioPlayHead must now override getPosition() instead
of getCurrentPosition(). Code that used to use the playhead's
CurrentPositionInfo must switch to using the new PositionInfo type.

**Rationale**

Not all hosts and plugin formats are capable of providing the full complement
of timing information contained in the old CurrentPositionInfo class.
Previously, in the case that some information could not be provided, fallback
values would be used instead, but it was not possible for clients to distinguish
between "real" values set explicitly by the host, and "fallback" values set by
a plugin wrapper. The new PositionInfo type keeps track of which members have
been explicitly set, so clients can implement their own fallback behaviour.
The new PositionInfo type also includes a new "barCount" member, which is
currently only used by the LV2 host and client.


## Change

The optional JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS preprocessor
flag will now use a new Metal layer renderer when running on macOS 10.14 or
later. The minimum requirements for building macOS and iOS software are now
macOS 10.13.6 and Xcode 10.1.

**Possible Issues**

Previously enabling JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS had no
negative effect on performance. Now it may slow rendering down.

**Workaround**

Disable JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS.

**Rationale**

JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS has been ineffective when
running on macOS 10.13 or later. Enabling this flag, and hence using the new
Metal layer renderer when running on macOS 10.14, restores the previous
behaviour and fixes problems where Core Graphics will render much larger
regions than necessary. However, the new renderer will may be slower than the
recently introduced default of asynchronous Core Graphics rendering, depending
on the regions that Core Graphcis is redrawing. Whether
JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS improves or degrades
performance is specific to an application.


## Change

The optional JUCE_COREGRAPHICS_DRAW_ASYNC preprocessor flag has been removed
and asynchronous Core Graphics rendering is now the default. The helper
function setComponentAsyncLayerBackedViewDisabled has also been removed.

**Possible Issues**

Components that were previously using setComponentAsyncLayerBackedViewDisabled
to conditionally opt out of asynchronous Core Graphics rendering will no longer
be able to do so.

**Workaround**

To opt out of asynchronous Core Graphics rendering the
windowRequiresSynchronousCoreGraphicsRendering ComponentPeer style flag can be
used when adding a component to the desktop.

**Rationale**

Asynchronous Core Graphics rendering provides a substantial performance
benefit. Asynchronous rendering is a property of a Peer, rather than a
Component, so a Peer style flag to conditionally opt out of asynchronous
rendering is more appropriate.


## Change

Constructors of AudioParameterBool, AudioParameterChoice, AudioParameterFloat,
AudioParameterInt, and AudioProcessorParameterWithID have been deprecated and
replaced with new constructors taking an 'Attributes' argument.

**Possible Issues**

The compiler may issue a deprecation warning upon encountering usages of the
old constructors.

**Workaround**

Update code to pass an 'Attributes' instance instead. Example usages of the new
constructors are given in the constructor documentation, and in the plugin
example projects.

**Rationale**

Parameter types have many different properties. Setting a non-default property
using the old constructors required explicitly setting other normally-defaulted
properties, which was redundant. The new Attributes types allow non-default
properties to be set in isolation.


# Version 6.1.6

## Change

Unhandled mouse wheel and magnify events will now be passed to the closest
enclosing enabled ancestor component.

**Possible Issues**

Components that previously blocked mouse wheel events when in a disabled state
may no longer block the events as expected.

**Workaround**

If a component should explicitly prevent events from propagating when disabled,
it should override mouseWheelMove() and mouseMagnify() to do nothing when the
component is disabled.

**Rationale**

Previously, unhandled wheel events would be passed to the parent component,
but only if the parent was enabled. This meant that scrolling on a component
nested inside a disabled component would have no effect by default. This
behaviour was not intuitive.


## Change

The invalidPressure, invalidOrientation, invalidRotation, invalidTiltX and
invalidTiltY members of MouseInputSource have been deprecated.

**Possible Issues**

Deprecation warnings will be seen when compiling code which uses these members
and eventually builds will fail when they are later removed from the API.

**Workaround**

Use the equivalent defaultPressure, defaultOrientation, defaultRotation,
defaultTiltX and defaultTiltY members of MouseInputSource.

**Rationale**

The deprecated members represent valid values and the isPressureValid() etc.
functions return true when using them. This could be a source of confusion and
may be inviting programming errors. The new names are in line with the ongoing
practice of using these values to provide a neutral default in the absence of
actual OS provided values.


## Change

Plugin wrappers will no longer call processBlockBypassed() if the wrapped
AudioProcessor returns a parameter from getBypassParameter().

**Possible Issues**

Plugins that used to depend on processBlockBypassed() being called may no
longer correctly enter a bypassed state.

**Workaround**

AudioProcessors that implement getBypassParameter() must check the current
value of the bypass parameter on each call to processBlock(), and bypass
processing as appropriate. When switching between bypassed and non-bypassed
states, the plugin must use some sort of ramping or smoothing to avoid
discontinuities in the output. If the plugin introduces latency when not
bypassed, the plugin must delay its output when in bypassed mode so that the
overall latency does not change when enabling/disabling bypass.

**Rationale**

The documentation for AudioProcessor::getBypassParameter() says
> if this method returns a non-null value, you should never call
  processBlockBypassed but use the returned parameter to control the bypass
  state instead.
Some plugin wrappers were not following this rule. After this change, the
behaviour of all plugin wrappers is consistent with the documented behaviour.


## Change

The ComponentPeer::getFrameSize() function has been deprecated on Linux.

**Possible Issues**

Deprecation warnings will be seen when compiling code which uses this function
and eventually builds will fail when it is later removed from the API.

**Workaround**

Use the ComponentPeer::getFrameSizeIfPresent() function. The new function returns
an OptionalBorderSize object. Use operator bool() to determine if the border size
is valid, then access the value using operator*() only if it is.

**Rationale**

The XWindow system cannot return a valid border size immediately after window
creation. ComponentPeer::getFrameSize() returns a default constructed
BorderSize<int> instance in such cases that corresponds to a frame size of
zero. That however can be a valid value, and needs to be treated differently
from the situation when the frame size is not yet available.


## Change

The return type of XWindowSystem::getBorderSize() was changed to
ComponentPeer::OptionalBorderSize.

**Possible Issues**

User code that uses XWindowSystem::getBorderSize() will fail to build.

**Workaround**

Use operator bool() to determine the validity of the new return value and
access the contained value using operator*().

**Rationale**

The XWindow system cannot immediately report the correct border size after
window creation. The underlying X11 calls will signal whether querying the
border size was successful, but there was no way to forward this information
through XWindowSystem::getBorderSize() until this change.


# Version 6.1.5

## Change

XWindowSystemUtilities::XSettings now has a private constructor.

**Possible Issues**

User code that uses XSettings::XSettings() will fail to build.

**Workaround**

Use the XSettings::createXSettings() factory function.

**Rationale**

The XSETTINGS facility is not available on all Linux distributions and the old
constructor would fail on such systems, potentially crashing the application.
The factory function will return nullptr in such situations instead.


# Version 6.1.3

## Change

The format specific structs of ExtensionsVisitor now return pointers to forward
declared types instead of `void*`. For this purpose the `struct AEffect;`
forward declaration was placed inside the global namespace.

**Possible Issues**

User code that includes the VST headers inside a namespace may fail to build,
because the forward declared type can collide with the contents of `aeffect.h`.

**Workaround**

The collision can be avoided by placing a `struct AEffect;` forward declaration
in the same namespace where the VST headers are included. The forward
declaration must come before the inclusion.

**Rationale**

Using the forward declared types eliminates the need for error prone casting
at the site where the ExtensionsVisitor facility is used.


## Change

ListBox::createSnapshotOfRows now returns ScaledImage instead of Image.

**Possible Issues**

User code that overrides this function will fail to build.

**Workaround**

To emulate the old behaviour, simply wrap the Image that was previous returned
into a ScaledImage and return that instead.

**Rationale**

Returning a ScaledImage allows the overriding function to specify the scale
at which the image should be drawn. Returning an oversampled image will provide
smoother-looking results on high resolution displays.


## Change

AudioFrameRate::frameRate is now a class type instead of an enum.

**Possible Issues**

Code that read the old enum value will not compile.

**Workaround**

Call frameRate.getType() to fetch the old enum type. Alternatively, use the new
getBaseRate(), isDrop(), isPullDown(), and getEffectiveRate() functions. The
new functions provide a more accurate description of the host's frame rate.

**Rationale**

The old enum-based interface was not flexible enough to describe all the frame
rates that might be reported by a plugin host.


## Change

FlexItem::alignSelf now defaults to "autoAlign" rather than "stretch".

**Possible Issues**

FlexBox layouts will be different in cases where FlexBox::alignItems is set to
a value other than "stretch". This is because each FlexItem will now default
to using the FlexBox's alignItems value. Layouts that explicitly set
FlexItem::alignSelf on each item will not be affected.

**Workaround**

To restore the previous layout behaviour, set FlexItem::alignSelf to "stretch"
on all FlexItems that would otherwise use the default value for alignSelf.

**Rationale**

The new behaviour more closely matches the behaviour of CSS FlexBox
implementations. In CSS, "align-self" has an initial value of "auto", which
computes to the parent's "align-items" value.


## Change

Functions on AudioPluginInstance that can add parameters have been made
private.

**Possible Issues**

Code implementing custom plugin formats may stop building if it calls these
functions.

**Workaround**

When implementing custom plugin formats, ensure that the plugin parameters
derive from AudioPluginInstance::HostedParameter and then use
addHostedParameter, addHostedParameterGroup or setHostedParameterTree to add
the parameters to the plugin instance.

**Rationale**

In a plugin host, it is very important to be able to uniquely identify
parameters across different versions of the same plugin. To make this possible,
we needed to introduce a way of retrieving a unique ID for each parameter,
which is now possible using the HostedParameter class. However, we also needed
to enforce that all AudioPluginInstances can only have parameters which are of
the type HostedParameter, which required hiding the old functions.


# Version 6.1.0

## Change

juce::gl::loadFunctions() no longer loads extension functions.

**Possible Issues**

Code that depended on extension functions being loaded automatically may cease
to function correctly.

**Workaround**

Extension functions can now be loaded using juce::gl::loadExtensions().

**Rationale**

There are a great number of extension functions, and on some systems these can
be slow to load (i.e. a second or so). Projects that do not require these
extension functions should not have to pay for this unnecessary overhead. Now,
only core functions will be loaded by default, and extensions can be loaded
explicitly in projects that require such functionality.


## Change

Thread::setPriority() will no longer set a realtime scheduling policy for all
threads with non-zero priorities on POSIX systems.

**Possible Issues**

Threads that implicitly relied on using a realtime policy will no longer
request a realtime policy if their priority is 7 or lower.

**Workaround**

For threads that require a realtime policy on POSIX systems, request a priority
of 8 or higher by calling Thread::setPriority() or
Thread::setCurrentThreadPriority().

**Rationale**

By default, new Thread instances have a priority of 5. Previously, non-zero
priorities corresponded to realtime scheduling policies, meaning that new
Threads would use the realtime scheduling policy unless they explicitly
requested a priority of 0. However, most threads do not and should not require
realtime scheduling. Setting a realtime policy on all newly-created threads may
degrade performance, as multiple realtime threads will end up fighting for
limited resources.


## Change

The JUCE_GLSL_VERSION preprocessor definition has been removed.

**Possible Issues**

Code which used this definition will no longer compile.

**Workaround**

Use OpenGLHelpers::getGLSLVersionString to retrieve a version string which is
consistent with the capabilities of the current OpenGL context.

**Rationale**

A compile-time version string is not very useful, as OpenGL versions and
capabilities can change at runtime. Replacing this macro with a function allows
querying the capabilities of the current context at runtime.


## Change

The minimum supported CMake version is now 3.15.

**Possible Issues**

It will no longer be possible to configure JUCE projects with CMake versions
between 3.12 and 3.14 inclusive.

**Workaround**

No workaround is available.

**Rationale**

Moving to 3.15 allows us to use target_link_directories and
target_link_options, which were introduced in 3.13, which in turn allows us to
provide support for bundled precompiled libraries in modules. Plugins already
required CMake 3.15, so this change just brings other target types in line with
the requirements for plugins.


## Change

The default value of JUCE_MODAL_LOOPS_PERMITTED has been changed from 1 to 0.

**Possible Issues**

With JUCE_MODAL_LOOPS_PERMITTED set to 0 code that previously relied upon modal
loops will need to be rewritten to use asynchronous versions of the modal
functions. There is no non-modal alternative to
AlterWindow::showNativeDialogBox and the previously modal behaviour of the
MultiDocumentPanel destructor has changed.

**Workaround**

Set JUCE_MODAL_LOOPS_PERMITTED back to 1.

**Rationale**

Modal operations are a frequent source of problems, particularly when used in
plug-ins. On Android modal loops are not possible, so people wanting to target
Android often have an unwelcome surprise when then have to rewrite what they
assumed to be platform independent code. Changing the default addresses these
problems.


## Change

The minimum supported C++ standard is now C++14 and the oldest supported
compilers on macOS and Linux are now Xcode 9.2, GCC 5.0 and Clang 3.4.

**Possible Issues**

Older compilers will no longer be able to compile JUCE. People using Xcode 8.5
on OS X 10.11 will need to update the operating system to OS X 10.12 to be able
to use Xcode 9.2.

**Workaround**

No workaround is available.

**Rationale**

This compiler upgrade will allow the use of C++14 within the framework.


## Change

Platform GL headers are no longer included in juce_opengl.h

**Possible Issues**

Projects depending on symbols declared in these headers may fail to build.

**Workaround**

The old platform-supplied headers have been replaced with a new juce_gl.h
header which is generated using the XML registry files supplied by Khronos.
This custom header declares GL symbols in the juce::gl namespace. If your code
only needs to be JUCE-compatible, you can explicitly qualify each name with
`juce::gl::`. If you need your code to build with different extension-loader
libraries (GLEW, GL3W etc.) you can make all GL symbols visible without
additional qualification with `using namespace juce::gl`.

**Rationale**

Using our own GL headers allows us to generate platform-independent headers
which include symbols for all specified OpenGL versions and extensions. Note
that although the function signatures exist, they may not resolve to a function
at runtime. If your code uses commands from an extension or recent GL version,
you should check each function pointer against `nullptr` before attempting to
use it. To avoid repeatedly checking, you could query a subset of functions
after calling gl::loadFunctions() and cache the results. Supplying custom GL
headers also allows us to use C++ techniques (namespaces, references), making
the headers safer than the platform-defined headers. Platform headers are
generally written in C, and export a significant portion of their symbols as
preprocessor definitions.


## Change

The functions `getComponentAsyncLayerBackedViewDisabled`
and `setComponentAsyncLayerBackedViewDisabled` were moved into the juce
namespace.

**Possible Issues**

Code that declares these functions may fail to link.

**Workaround**

Move declarations of these functions into the juce namespace.

**Rationale**

Although the names of these functions are unlikely to collide with functions
from other libraries, we can make such collisions much more unlikely by keeping
JUCE code in the juce namespace.


## Change

The `juce_blocks_basics` module was removed.

**Possible Issues**

Projects depending on `juce_blocks_basics` will not build.

**Workaround**

The BLOCKS API is now located in a separate repository:
https://github.com/WeAreROLI/roli_blocks_basics
Projects which used to depend on `juce_blocks_basics` can use
`roli_blocks_basics` instead.

**Rationale**

ROLI is no longer involved with the development of JUCE. Therefore, development
on the BLOCKS API has been moved out of the JUCE repository, and to a new
repository managed by ROLI.


## Change

The live build functionality of the Projucer has been removed.

**Possible Issues**

You will no longer be able to use live build in the Projucer.

**Workaround**

None.

**Rationale**

Keeping the live build compatible with the latest compilers on all our
supported platforms is a very substantial maintenance burden, but very few
people are using this feature of the Projucer. Removing the live build will
simplify the code and our release process.


## Change

`Component::createFocusTraverser()` has been renamed to
`Component::createKeyboardFocusTraverser()` and now returns a `std::unique_ptr`
instead of a raw pointer. `Component::createFocusTraverser()` is a new method
for controlling basic focus traversal and not keyboard focus traversal.

**Possible Issues**

Derived Components that override the old method will no longer compile.

**Workaround**

Override the new method. Be careful to override
`createKeyboardFocusTraverser()` and not `createFocusTraverser()` to ensure
that the behaviour is the same.

**Rationale**

The ownership of this method is now clearer as the previous code relied on the
caller deleting the object. The name has changed to accommodate the new
`Component::createFocusTraverser()` method that returns an object for
determining basic focus traversal, of which keyboard focus is generally a
subset.


## Change

PluginDescription::uid has been deprecated and replaced with a new 'uniqueId'
data member.

**Possible Issues**

Code using the old data member will need to be updated in order to compile.

**Workaround**

Code that used to use 'uid' to identify plugins should switch to using
'uniqueId', with some caveats - see "Rationale" for details.

**Rationale**

The 'uniqueId' member has the benefit of being consistent for
a given VST3 across Windows, macOS, and Linux. However, the value of the
uniqueId may differ from the value of the old uid on some platforms. The value
of the old 'uid' member can now be found in the 'deprecatedUid' member, which
should allow clients to implement logic such as checking a saved uid against
the new uniqueId, and falling back to the deprecatedUid. This should allow
hosts to gracefully upgrade from the old uid values to the new values.


# Version 6.0.8

## Change

Calling AudioProcessorEditor::setResizeLimits() will no longer implicitly add a
ResizableCornerComponent to the editor if it has not already been set as
resizable.

**Possible Issues**

Code which previously relied on calling this method to set up the corner
resizer will no longer work.

**Workaround**

Explicitly call AudioProcessorEditor::setResizable() with the second argument
set to true to enable the corner resizer.

**Rationale**

The previous behaviour was undocumented and potentially confusing. There is now
a single method to control the behaviour of the editor's corner resizer to
avoid any ambiguity.


## Change

The implementations of `getValue` and `setValue` in `AUInstanceParameter` now
properly take the ranges of discrete parameters into account.

**Possible Issues**

This issue affects JUCE Audio Unit hosts. Automation data previously saved for
a discrete parameter with a non-zero minimum value may not set the parameter to
the same values as previous JUCE versions. Note that previously, `getValue` on
a hosted discrete parameter may have returned out-of-range values, and
`setValue` may have only mapped to a portion of the parameter range. As a
result, automation recorded for affected parameters was likely already behaving
unexpectedly.

**Workaround**

There is no workaround.

**Rationale**

The old behaviour was incorrect, and was causing issues in plugin validators
and other hosts. Hosts expect `getValue` to return a normalised parameter
value. If this function returns an out-of-range value (including Inf and NaN)
this is likely to break assumptions made by the host, leading to crashes,
corrupted project data, or other defects.


## Change

AudioProcessorListener::audioProcessorChanged gained a new parameter describing
the nature of any change.

**Possible Issues**

Code using the old function signature will not build until updated to use
the new signature.

**Workaround**

Listeners should add the new parameter to any overrides of
audioProcessorChanged.

**Rationale**

The new function signature means that wrappers can be smarter about the
requests that they make to hosts whenever some aspect of the processor changes.
In particular, plugin wrappers can now distinguish between changes to latency,
parameter attributes, and the current program. This means that hosts will no
longer assume parameters have changed when `setLatencySamples` is called.


## Change

CharacterFunctions::readDoubleValue now returns values consistent with other
C++ number parsing libraries. Parsing values smaller than the minimum number
representable in a double will return (+/-)0.0 and parsing values larger than
the maximum number representable in a double will return (+/-)inf.

**Possible Issues**

Code reading very large or very small numbers may receive values of 0.0 and inf
rather than nan.

**Workaround**

Where you may be using std::isnan to check the validity of the result you can
instead use std::isfinite.

**Rationale**

The new behaviour is consistent with other string parsing libraries.


# Version 6.0.6

## Change

The name of `OperatingSystemType::MacOSX_11_0` was changed to
`OperatingSystemType::MacOS_11`.

**Possible Issues**

Code using the old name will not build until it is updated to use the new name.

**Workaround**

Update code using the old name to use the new name instead.

**Rationale**

Newer versions of macOS have dropped the "X" naming. Minor version updates are
also less significant now than they were for the X-series.


## Change

Xcode projects generated using the Projucer will now use the "New Build System"
instead of the "Legacy Build System" by default.

**Possible Issues**

Xcode 10.0 - 10.2 has some known issues when using the new build system such as
JUCE modules not rebuilding correctly when modified, issue and file navigation
not working, and breakpoints not being reliably set or hit.

**Workaround**

If you are using an affected version of Xcode then you can enable the "Use
Legacy Build System" setting in the Projucer Xcode exporter to go back to the
previous behaviour.

**Rationale**

The legacy build system has issues building arm64 binaries for Apple silicon
and will eventually be removed altogether.


# Version 6.0.5

## Change

New pure virtual methods accepting `PopupMenu::Options` arguments have been
added to `PopupMenu::LookAndFeelMethods`.

**Possible Issues**

Classes derived from `PopupMenu::LookAndFeelMethods`, such as custom
LookAndFeel classes, will not compile unless these pure virtual methods are
implemented.

**Workaround**

The old LookAndFeel methods still exist, so if the new Options parameter is not
useful in your application, your implementation of
`PopupMenu::LookAndFeelMethods` can simply forward to the old methods. For
example, your implementation of `drawPopupMenuBackgroundWithOptions` can
internally call your existing `drawPopupMenuBackground` implementation.

**Rationale**

Allowing the LookAndFeelMethods to access the popup menu's options allows for
more flexible styling. For example, a theme may wish to query the menu's target
component or parent for colours to use.


## Change

A typo in the JUCEUtils CMake script that caused the wrong manufacturer code to
be set in the compile definitions for a plugin was fixed.

**Possible Issues**

The manufacturer code for plugins built under CMake with this version of JUCE
will differ from the manufacturer code that was generated previously.

**Workaround**

If you have released plugins that used the old, incorrect manufacturer code and
wish to continue using this code for backwards compatibility, add the following
to your `juce_add_plugin` call:

    USE_LEGACY_COMPATIBILITY_PLUGIN_CODE TRUE

In most cases, this should not be necessary, and we recommend using the fixed
behaviour.

**Rationale**

This change ensures that the manufacturer codes used by CMake projects match
the codes that would be generated by the Projucer, improving compatibility
when transitioning from the Projucer to CMake.


# Version 6.0.2

## Change

The JUCE_WASAPI_EXCLUSIVE flag has been removed from juce_audio_devices and all
available WASAPI audio device modes (shared, shared low latency and exclusive)
are available by default when JUCE_WASAPI is enabled. The
AudioIODeviceType::createAudioIODeviceType_WASAPI() method which takes a single
boolean argument has also been deprecated in favour of a new method which takes
a WASAPIDeviceMode enum.

**Possible Issues**

Code that relied on the JUCE_WASAPI_EXCLUSIVE flag to disable WASAPI exclusive
mode will no longer work.

**Workaround**

Override the AudioDeviceManager::createAudioDeviceTypes() method to omit the
WASAPI exclusive mode device if you do not want it to be available.

**Rationale**

JUCE now supports shared low latency WASAPI audio devices via the AudioClient3
interface and instead of adding an additional compile time config flag to
enable this functionality, which adds complexity to the build process when not
using the Projucer, JUCE makes all WASAPI device modes available by default.


## Change

The fields representing Mac OS X 10.4 to 10.6 inclusive have been removed from
the `OperatingSystemType` enum.

**Possible Issues**

Code that uses these fields will fail to build.

**Workaround**

Remove references to these fields from user code.

**Rationale**

JUCE is not supported on Mac OS X versions lower than 10.7, so it is a given
that `getOperatingSystemType` will always return an OS version greater than or
equal to 10.7. Code that changes behaviours depending on the OS version can
assume that this version is at least 10.7.


## Change

The JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING flag in juce_graphics is no
longer used on iOS.

**Possible Issues**

Projects with this flag enabled may render differently on iOS.

**Workaround**

There is no workaround.

**Rationale**

When using a cached image to render Components with `setBufferedToImage (true)`
the result now matches the default behaviour on iOS where fonts are not
smoothed.


## Change

Space, return and escape key events on the native macOS menu bar are no longer
passed to the currently focused JUCE Component.

**Possible Issues**

Code relying on receiving these keyboard events will no longer work.

**Workaround**

There is no workaround.

**Rationale**

It should be possible for users with a keyboard or assistive device to navigate
the menu, invoking the currently highlighted menu item with the space or return
key and dismissing the menu with the escape key. These key events should not be
passed to the application and doing so interferes with the accessibility of
JUCE apps. Only passing these events to the native macOS menu means that JUCE
apps behave as expected for users.


# Version 6.0.0

## Change

The Convolution class interface was changed:
- `loadImpulseResponse` member functions now take `enum class` parameters
  instead of `bool`.
- `copyAndLoadImpulseResponseFromBlock` and
  `copyAndLoadImpulseResponseFromBuffer` were replaced by a new
  `loadImpulseResponse` overload.

**Possible Issues**

Code using the old interface will no longer compile, and will need to be
updated.

**Workaround**

Code that was previously loading impulse responses from binary data or from
files can substitute old `bool` parameters with the newer `enum class`
equivalents. Code that was previously passing buffers or blocks will need
reworking so that the Convolution instance can take ownership of the buffer
containing the impulse response.

**Rationale**

The newer `enum class` parameters make user code much more readable, e.g.
`loadImpulseResponse (file, Stereo::yes, Trim::yes, 0, Normalise::yes)` rather
than `loadImpulseResponse (file, true, true, 0, true);`. By taking ownership of
the passed buffer, the Convolution can avoid preallocating a large internal
buffer, reducing memory usage when short impulse responses are used. Changing
the ownership semantics of the buffer also makes it easier for users to avoid
copies/allocations on the audio thread, and gives more flexibility to the
implementation to run initialisation tasks on a background thread.


## Change

All references to ROLI Ltd. (ROLI) have been changed to Raw Material Software
Limited.

**Possible Issues**

Existing projects, particularly Android, may need to be resaved by the Projucer
and have the old build artefacts deleted before they will build.

**Workaround**

In Android projects any explicit mention of paths with the from "com.roli.*"
should be changed to the form "com.rmsl.*".

**Rationale**

This change reflects the change in ownership from ROLI to RMSL.


## Change

The Windows DPI handling in the VST wrapper and hosting code has been
refactored to be more stable.

**Possible Issues**

The new code uses a top-level AffineTransform to scale the JUCE editor window
instead of native methods. Therefore any AudioProcessorEditors which have their
own AffineTransform applied will no longer work correctly.

**Workaround**

If you are using an AffineTransform to scale the entire plug-in window then
consider putting the component you want to transform in a child of
the editor and transform that instead. Alternatively, if you don't need a
separate scale factor for each plug-in instance you can use
Desktop::setGlobalScaleFactor().

**Rationale**

The old code had some bugs when using OpenGL and when moving between monitors
with different scale factors. The new code should fix these and DPI-aware
plug-ins will scale correctly.


## Change

Relative Xcode subproject paths specified in the Projucer are now relative to
the build directory rather than the project directory.

**Possible Issues**

After being re-saved in the Projucer existing Xcode projects will fail to find
any subprojects specified using a relative path.

**Workaround**

Update the subproject path in the Projucer.

**Rationale**

Most other Xcode specific paths are specified relative to the build directory.
This change brings the Xcode subproject path in line with the rest of the
configuration.


# Version 5.4.6

## Change

AudioProcessorValueTreeState::getRawParameterValue now returns a
std::atomic<float>* instead of a float*.

**Possible Issues**

Existing code which explicitly mentions the type of the returned value, or
interacts with the dereferenced float in ways unsupported by the std::atomic
wrapper, will fail to compile. Certain evaluation-reordering compiler
optimisations may no longer be possible.

**Workaround**

Update your code to deal with a std::atomic<float>* instead of a float*.

**Rationale**

Returning a std::atomic<float>* allows the JUCE framework to have much stronger
guarantees about thread safety.


## Change

Removed a workaround from the ASIOAudioIODevice::getOutputLatencyInSamples()
and ASIOAudioIODevice::getInputLatencyInSamples() methods which was adding an
arbitrary amount to the reported latencies to compensate for dodgy, old
drivers.

**Possible Issues**

Code which relied on these altered values may now behave differently.

**Workaround**

Update your code to deal with the new, correct values reported from the drivers
directly.

**Rationale**

JUCE will now return the latency values as reported by the drivers without
adding anything to them. The workaround was for old drivers and the current
drivers should report the correct values without the need for the workaround.


## Change

The default behaviour of the AU and AUv3 plug-in wrappers is now to call
get/setStateInformation instead of get/setProgramStateInformation.

**Possible Issues**

AudioProcessor subclasses which have overridden the default implementations of
get/setProgramStateInformation (which simply call through to
get/setStateInformation) may be unable to load previously saved state; state
previously saved via a call to getProgramStateInformation will be presented to
setStateInformation.

**Workaround**

Enable the JUCE_AU_WRAPPERS_SAVE_PROGRAM_STATES configuration option in the
juce_audio_plugin_client module to preserve backwards compatibility if
required.

**Rationale**

When using overridden get/setProgramStateInformation methods the previous
behaviour of the AU and AUv3 wrappers does not correctly save and restore
state.


# Version 5.4.5

## Change

The alignment of text rendered on macOS using CoreGraphics may have shifted
slightly, depending on the font you have used. The default macOS font has
shifted downwards.

**Possible Issues**

Meticulously aligned text components of a GUI may now be misaligned.

**Workaround**

Use a custom LookAndFeel to change the location where text is drawn, or use a
different font that matches the previous alignment of your original font.

**Rationale**

This was an unintentional change resulting from moving away from a deprecated
macOS text API. The new alignment is consistent with other rendering engines
(web browsers and text editors) and the software renderer.


## Change

The JUCEApplicationBase::backButtonPressed() method now returns a bool to
indicate whether the back event was handled or not.

**Possible Issues**

Applications which override this method will fail to compile.

**Workaround**

You will need to update your code to return a bool indicating whether the back
event was handled or not.

**Rationale**

The back button behaviour on Android was previously broken as it would not do
anything. The new code will correctly call finish() on the Activity when the
back button is pressed but this method now allows the user to override this to
implement their own custom navigation behaviour by returning true to indicate
that it has been handled.


## Change

The AudioBlock class has been refactored and some of the method names have
changed. Additionally the `const` behaviour now mirrors that of `std::span`,
with the `const`-ness of the contained data decoupled from the `const`-ness of
the container.

**Possible Issues**

Code using the old method names or violating `const`-correctness will fail to
compile.

**Workaround**

You will need to update your code to use the new method names and select an
appropriate `const`-ness for the AudioBlock and the data it references.

**Rationale**

The names of some of the methods in the AudioBlock class were ambiguous,
particularly when chaining methods involving references to other blocks. The
interaction between the `const`-ness of the AudioBlock and the `const`-ness of
the referenced data was also ambiguous and has now been standardised to the
same behaviour as other non-owning data views like `std::span`.


# Version 5.4.4

## Change

The Visual Studio 2013 exporter has been removed from the Projucer and we will
no longer maintain backwards compatibility with Visual Studio 2013 in JUCE.

**Possible Issues**

It is no longer possible to create Visual Studio 2013 projects from the
Projucer or compile JUCE-based software using Visual Studio 2013.

**Workaround**

If you are using Visual Studio 2013 to build your projects you will need to
update to a more modern version of Visual Studio.

**Rationale**

Of all the platforms JUCE supports Visual Studio 2013 was holding us back the
most in terms of C++ features we would like to use more broadly across the
codebase. It is still possible to target older versions of Windows with more
modern versions of Visual Studio. Until recently the AAX SDK was distributed as
a Visual Studio 2013 project, but this is now provided as a Visual Studio 2017
project.


## Change

JUCE is moving towards using C++11 pointer container types instead of passing
raw pointers as arguments and return values.

**Possible Issues**

You will need to change your code to pass std::unique_ptr into and out of
various functions across JUCE's API.

**Workaround**

None

**Rationale**

Indicating ownership through the transfer of smart pointer types has been part
of mainstream C++ for a long time and this change enforces memory safety by
default in most situations.


## Change

SystemTrayIconComponent::setIconImage now takes two arguments, rather than one.
The new argument is a template image for use on macOS where all non-transparent
regions will render in a monochrome colour determined dynamically by the
operating system.

**Possible Issues**

You will now need to provide two images to display a SystemTrayIconComponent
and the SystemTrayIconComponent will have a different appearance on macOS.

**Workaround**

If you are not targeting macOS then you can provide an empty image, `{}`, for
the second argument. If you are targeting macOS then you will likely need to
design a new monochrome icon.

**Rationale**

The introduction of "Dark Mode" in macOS 10.14 means that menu bar icons must
support several different colours and highlight modes to retain the same
appearance as the native Apple icons. Doing this correctly without delegating
the behaviour to the operating system is extremely cumbersome, and the APIs we
were previously using to interact with menu bar items have been deprecated.


## Change

The AudioBlock class now differentiates between const and non-const data.

**Possible Issues**

The return type of the getInputBlock() method of the ProcessContextReplacing
and ProcessContextNonReplacing classes has changed from AudioBlock<X> to
AudioBlock<const X>.

**Workaround**

For ProcessContextReplacing you should use getOutputBlock() instead of
getInputBlock(). For ProcessContextNonReplacing attempting to modify the input
block is very likely an error.

**Rationale**

This change makes the intent of the code much clearer and means that we can
remove some const_cast operations.


## Change

The formatting of floating point numbers written to XML and JSON files has
changed.

Note that there is no change in precision - the XML and JSON files containing
the new format numbers will parse in exactly the same way, it is only the
string representation that has changed.

**Possible Issues**

If you rely upon exactly reproducing XML or JSON files then the new files may
be different.

**Workaround**

Update any reference XML or JSON files to use the new format.

**Rationale**

The new format retains full precision, provides a human friendly representation
of values near 1, and uses scientific notation for small and large numbers.
This prevents needless file size bloat from numbers like 0.00000000000000001.


# Version 5.4.3

## Change

The global user module path setting in the Projucer can now only contain a
single path.

**Possible Issues**

Projects that previously relied on using multiple global user module paths
separated by a semicolon will fail to find these modules after re-saving.

**Workaround**

Replace the multiple paths with a single global user module path.

**Rationale**

Using multiple global user module paths did not work when saving a project
which exported to different OSes. Only allowing a single path will prevent this
from silently causing issues.


# Version 5.4.2

## Change

The return type of Block::getBlockAreaWithinLayout() has been changed from
Rectangle to a simpler BlockArea struct.

**Possible Issues**

Classes that derive from Block and implement this pure virtual method will no
longer compile due to a change in the function signature.

**Workaround**

Update the method to return a BlockArea struct and update code that calls
getBlockAreaWithinLayout to handle a BlockArea instead of a Rectangle.

**Rationale**

The juce_blocks_basics is ISC licensed and therefore cannot depend on the
GPL/Commercial licensed juce_graphics module that contains Rectangle.


## Change

Renaming and deletion of open file handles on Windows is now possible using the
FILE_SHARE_DELETE flag.

**Possible Issues**

Previous code that relied on open files not being able to be renamed or deleted
on Windows may fail.

**Workaround**

No workaround.

**Rationale**

This unifies the behaviour across OSes as POSIX systems already allow this.


## Change

Multiple changes to low-level, non-public JNI and Android APIs.

**Possible Issues**

If you were using any non-public, low-level JNI macros, calling java code or
receiving JNI callbacks, then your code will probably no longer work. See the
forum for further details.

**Workaround**

See the forum for further details.

**Rationale**

See the forum for further details.


## Change

The minimum Android version for a JUCE app is now Android 4.1

**Possible Issues**

Your app may not run on very old versions of Android  (less than 0.5% of the
devices).

**Workaround**

There is no workaround.

**Rationale**

Less than 0.5% of all devices in the world run versions of Android older than
Android 4.1. In the interest of keeping JUCE code clean and lean, we must
deprecate support for very old Android versions from time to time.


# Version 5.4.0

## Change

The use of WinRT MIDI functions has been disabled by default for any version
of Windows 10 before 1809 (October 2018 Update).

**Possible Issues**

If you were previously using WinRT MIDI functions on older versions of Windows
then the new behaviour is to revert to the old Win32 MIDI API.

**Workaround**

Set the preprocessor macro JUCE_FORCE_WINRT_MIDI=1 (in addition to the
previously selected JUCE_USE_WINRT_MIDI=1) to allow the use of the WinRT API on
older versions of Windows.

**Rationale**

Until now JUCE's support for the Windows 10 WinRT MIDI API was experimental,
due to longstanding issues within the API itself. These issues have been
addressed in the Windows 10 1809 (October 2018 Update) release.


## Change

The VST2 SDK embedded within JUCE has been removed.

**Possible Issues**

1. Building or hosting VST2 plug-ins requires header files from the VST2 SDK,
   which is no longer part of JUCE.
2. Building a VST2-compatible VST3 plug-in (the previous default behaviour in
   JUCE) requires header files from the VST2 SDK, which is no longer part of
   JUCE.

**Workaround**

1. The VST2 SDK can be obtained from the vstsdk3610_11_06_2018_build_37 (or
   older) VST3 SDK or JUCE version 5.3.2. You should put the VST2 SDK in your
   header search paths or use the "VST (Legacy) SDK Folder" fields in the
   Projucer.
2. For new plug-in projects where you will be releasing both a VST2 and VST3
   version, and you want the VST3 plug-in to replace the VST2 plug-in in
   hosts that support it, then you should enable the JUCE_VST3_CAN_REPLACE_VST2
   option.
3. When a new JUCE plug-in project is created the value of
   JUCE_VST3_CAN_REPLACE_VST2 will be set to zero.

**Rationale**

Distributing VST2 plug-ins requires a VST2 license from Steinberg. Following
Steinberg's removal of the VST2 SDK from their public SDKs we are also removing
the VST2 SDK from the JUCE codebase.


## Change

The AudioProcessorValueTreeState::createAndAddParameter function has been
deprecated.

**Possible Issues**

Deprecation warnings will be seen when compiling code which uses this function
and eventually builds will fail when it is later removed from the API.

**Workaround**

Previous calls to

createAndAddParameter (paramID, paramName, ...);

can be directly replaced with

using Parameter = AudioProcessorValueTreeState::Parameter;
createAndAddParameter (std::make_unique<Parameter> (paramID, paramName, ...));

but an even better approach is to use the new AudioProcessorValueTreeState
constructor where you can pass both RangedAudioParameters and
AudioProcessorParameterGroups of RangedAudioParameters to the
AudioProcessorValueTreeState and initialise the ValueTree simultaneously.

**Rationale**

The new createAndAddParameter method is much more flexible and enables any
parameter types derived from RangedAudioParameter to be managed by the
AudioProcessorValueTreeState.


## Change

The Projucer's per-exporter Android SDK/NDK path options have been removed.

**Possible Issues**

Projects that previously used these fields may no longer build.

**Workaround**

Use the Projucer's global paths settings to point to these directories, either
by opening the "Projucer/File->Global Paths..." menu item or using the
"--set-global-search-path" command-line option.

**Rationale**

Having multiple places where the paths could be set was confusing and could
cause unexpected mismatches.


## Change

SystemStats::getDeviceDescription() will now return the device code on iOS e.g.
"iPhone7, 2" for an iPhone 6 instead of just "iPhone".

**Possible Issues**

Code that previously relied on this method returning either explicitly "iPhone"
or "iPad" may no longer work.

**Workaround**

Modify this code to handle the new device code string e.g. by changing:
SystemStats::getDeviceDescription() == "iPhone";
to
SystemStats::getDeviceDescription().contains ("iPhone");.

**Rationale**

The exact device model can now be deduced from this information instead of just
the device family.


## Change

DragAndDropContainer::performExternalDragDropOfFiles() and
::performExternalDragDropOfText() are now asynchronous on Windows.

**Possible Issues**

Code that previously relied on these operations being synchronous and blocking
until completion will no longer work as the methods will return immediately and
run asynchronously.

**Workaround**

Use the callback argument that has been added to these methods to register a
lambda that will be called when the operation has been completed.

**Rationale**

The behaviour of these methods is now consistent across all platforms and the
method no longer blocks the message thread on Windows.


## Change

AudioProcessor::getTailLengthSeconds can now return infinity for
VST/VST3/AU/AUv3.

**Possible Issues**

If you are using the result of getTailLengthSeconds to allocate a buffer in
your host, then your host will now likely crash when loading a plug-in with an
infinite tail time.

**Workaround**

Rewrite your code to not use the result of getTailLengthSeconds directly to
allocate a buffer.

**Rationale**

Before this change there was no way for a JUCE plug-in to report an infinite
tail time.


# Version 5.3.2

## Change

The behaviour of an UndoManager used by an AudioProcessorValueTreeState has
been improved.

**Possible Issues**

If your plug-in contains an UndoManager used by an AudioProcessorValueTreeState
and relies upon the old behaviour of the UndoManager then it is possible that
the new behaviour is no longer appropriate for your use case.

**Workaround**

Use an external UndoManager to reproduce the old behaviour manually.

**Rationale**

This change fixes a few bugs in the behaviour of an UndoManager used by an
AudioProcessorValueTreeState.


## Change

JUCE no longer supports OS X deployment targets earlier than 10.7.

**Possible Issues**

If you were previously targeting OS X 10.5 or 10.6 you will no longer be able
to build JUCE-based products compatible with those platforms.

**Workaround**

None. With the appropriate JUCE licence you may be able to backport new JUCE
features, but there will be no official support for this.

**Rationale**

Increasing the minimum supported OS X version allows the JUCE codebase to make
use of the more modern C++ features found in the 10.7 standard library, which
in turn will increase thread and memory safety.


# Version 5.3.0

## Change

The JUCE examples have been cleaned up, modernised and converted into PIPs
(Projucer Instant Projects). The JUCE Demo has been removed and replaced by the
DemoRunner application and larger projects such as the Audio Plugin Host and
the Network Graphics Demo have been moved into the extras directory.

**Possible Issues**

1. Due to the large number of changes that have occurred in the JUCE Git
   repository, pulling this version may result in a messy folder structure with
   empty directories that have been removed.
2. The JUCE Demo project is no longer in the JUCE repository.
3. The Audio Plugin Host project has moved from the examples directory to the
   extras directory.

**Workaround**

1. Run a Git clean command (git clean -xdf) in your JUCE directory to remove
   all untracked files, directories and build products.
2. The new DemoRunner application, located in extras/DemoRunner, can be used to
   preview all the JUCE examples and see the code side-by-side.
3. Change any file paths that depended on the plugin host project being located
   in the examples directory to use the extras directory instead.

**Rationale**

The JUCE examples had inconsistent naming, coding styles and the projects and
build products took up a large amount of space in the repository. Replacing
them with PIPs reduces the file size and allows us to categorise the examples
better, as well as cleaning up the code.


## Change

When hosting plug-ins all AudioProcessor methods of managing parameters that
take a parameter index as an argument have been deprecated.

**Possible Issues**

A single assertion will be fired in debug builds on the first use of a
deprecated function.

**Workaround**

When hosting plug-ins you should use the AudioProcessor::getParameters() method
and interact with parameters via the returned array of
AudioProcessorParameters. For a short-term fix you can also continue past the
assertion in your debugger, or temporarily modify the JUCE source code to
remove it.

**Rationale**

Given the structure of JUCE's API it is impossible to deprecate these functions
using only compile-time messages. Therefore a single assertion, which can be
safely ignored, serves to indicate that these functions should no longer be
used. The move away from the AudioProcessor methods both improves the interface
to that class and makes ongoing development work much easier.


## Change

This InAppPurchases class is now a JUCE Singleton. This means that you need
to get an instance via InAppPurchases::getInstance(), instead of storing a
InAppPurchases object yourself.

**Possible Issues**

Any code using InAppPurchases needs to be updated to retrieve a singleton
pointer to InAppPurchases.

**Workaround**

Instead of holding a InAppPurchase member yourself, you should get an instance
via InAppPurchases::getInstance(), e.g.

instead of:

InAppPurchases iap;
iap.purchaseProduct (...);

call:

InAppPurchases::getInstance()->purchaseProduct (...);

**Rationale**

This change was required to fix an issue on Android where on failed transaction
a listener would not get called.


## Change

JUCE's MPE classes have been updated to reflect the official specification
recently approved by the MIDI Manufacturers Association (MMA).

**Possible Issues**

The most significant changes have occurred in the MPEZoneLayout classes and
programs using the higher level MPE classes such as MPEInstrument,
MPESynthesiser, MPESynthesiserBase and MPESynthesiserVoice should be
unaffected.

Previously, any MIDI channel from 1 - 15 could be selected to be the master
channel of an MPE zone, with a specified number of member channels ascending
from the master channel + 1. However, in the new specification this has been
simplified so that a device only has a lower and/or an upper zone, where the
lower zone has master channel 1 and assigns new member channels ascending from
channel 2 and the upper zone has master channel 16 and assigns new member
channels descending from channel 15.

**Workaround**

Use the MPEZoneLayout::setLowerZone() and MPEZoneLayout::setUpperZone() methods
to set zone layouts.

Any UI that allows users to select and set zones on an MPE instrument should
also be updated to reflect the specification changes.

**Rationale**

The MPE classes in JUCE are out of date and should be updated to reflect the
new, official MPE standard.


# Version 5.2.1

## Change

Calling JUCEApplicationBase::quit() on Android will now really quit the app,
rather than just placing it in background. Starting with API level 21 (Android
5.0), the app will not appear in recent apps list after calling quit(). Prior
to API 21, the app will still appear in recent app lists but when a user
chooses the app, a new instance of the app will be started.

**Possible Issues**

Any code calling JUCEApplicationBase::quit() to place the app in background
will close the app instead.

**Workaround**

Use Process::hide().

**Rationale**

The old behaviour JUCEApplicationBase::quit() was confusing JUCE code, as a new
instance of JUCE app was attempted to be created, while the older instance was
still running in background. This would result in assertions when starting a
second instance.


## Change

On Windows, release builds will now link to the dynamic C++ runtime by default

**Possible Issues**

If you are creating a new .jucer project, then your plug-in will now link to
the dynamic C++ runtime by default, which means that you MUST ensure that the
C++ runtime libraries exist on your customer's computers.

**Workaround**

If you are only targeting Windows 10, then the C++ runtime is now part of the
system core components and will always exist on the computers of your customers
(just like kernel332.dll, for example). If you are targeting Windows versions
between Vista and Windows 10, then you should build your plug-in with the
latest updated version of VS2015 or later, which ensures that it's linked to
the universal runtime. Universal runtime is part of the system's core libraries
on Windows 10 and on Windows versions Vista to 8.1, it will be available on
your customer's computers via Windows Update. Unfortunately, if your customer
has just installed Windows 8.1 to Vista on a fresh computer, then there is a
chance that the update mechanism for the universal runtime hasn't triggered yet
and your plug-in may still fail. Your installer should prompt the user to
install all the Windows updates in this case or you can deploy the universal
runtime as a redistributable with your installer. If you are targeting earlier
versions of Windows then you should always include the runtime as a
redistributable with your plug-in's installer. Alternatively, you can change
the runtime linking to static (however, see 'Rationale' section).

**Rationale**

In a recent update to Windows 10, Microsoft has limited the number of fiber
local storage (FLS) slots per process. Effectively, this limits how many
plug-ins with static runtime linkage can be loaded into a DAW. In the worst
case, this limits the total number of plug-ins to a maximum of 64 plug-ins.
There is no workaround for DAW vendors and the only solution is to push plug-in
vendors to use the dynamic runtime. To help with this, JUCE has decided to make
dynamic runtime linkage the default in JUCE.


## Change

AudioProcessorGraph interface has changed in a number of ways - Node objects
are now reference counted, there are different accessor methods to iterate
them, and misc other small improvements to the API

**Possible Issues**

The changes won't cause any silent errors in user code, but will require some
manual refactoring

**Workaround**

Just find equivalent new methods to replace existing code.

**Rationale**

The graph class was extremely old and creaky, and these changes is the start of
an improvement process that should eventually result in it being broken down
into fundamental graph building block classes for use in other contexts.


# Version 5.2.0

## Change

Viewport now enables "scroll on drag" mode by default on Android and iOS.

**Possible Issues**

Any code relying on "scroll on drag" mode being turned off by default, should
disable it manually.

**Workaround**

None.

**Rationale**

It is expected on mobile devices to be able to scroll a list by just a drag,
rather than using a dedicated scrollbar. The scrollbar is still available
though if needed.


## Change

The previous setting of Android exporter "Custom manifest xml elements"
creating child nodes of <application> element has been replaced by "Custom
manifest XML content" setting that allows to specify the content of the entire
manifest instead.  Any previously values of the old setting will be used in the
new setting by default, and they will need changing as mentioned in Workaround.
The custom content will be merged with the content auto-generated by Projucer.
Any custom elements or custom attributes will override the ones set by
Projucer. Projucer will also automatically add any missing and required
elements and attributes.

**Possible Issues**

If a Projucer project used "Custom manifest xml elements" field, the value will
no longer be compatible with the project generated in the latest Projucer
version. The solution is very simple and quick though, as mentioned in the
Workaround section.

**Workaround**

For any elements previously used, simply embed them explicitly in
<manifest><application> elements, for example instead of:

<meta-data android:name="paramId1" android:value="paramValue1"/>
<meta-data android:name="paramId2" android:value="paramValue2"/>

simply write:

<manifest>
<application>
<meta-data android:name="paramId1" android:value="paramValue1"/>
<meta-data android:name="paramId2" android:value="paramValue2"/>
</application>
</manifest>

**Rationale**

To maintain the high level of flexibility of generated Android projects and to
avoid creating fields in Projucer for every possible future parameter, it is
simpler to allow to set up the required parameters manually. This way it is not
only possible to add any custom elements but it is also possible to override
the default attributes assigned by Projucer for the required elements. For
instance, if the default value of <supports-screens> element is not
satisfactory because you want a support for x-large screens only, simply set
"Custom manifest XML content" to:

<manifest>
<supports-screens android:xlargeScreens="true"/>
</manifest>


# Version 5.1.2

## Change

The method used to classify AudioUnit, VST3 and AAX plug-in parameters as
either continuous or discrete has changed, and AudioUnit and AudioUnit v3
parameters are marked as high precision by default.

**Possible Issues**

Plug-ins: DAW projects with automation data written by an AudioUnit, AudioUnit
v3 VST3 or AAX plug-in built with JUCE version 5.1.1 or earlier may load
incorrectly when opened by an AudioUnit, AudioUnit v3, VST3 or AAX plug-in
built with JUCE version 5.1.2 and later.

Hosts: The AudioPluginInstance::getParameterNumSteps method now returns correct
values for AU and VST3 plug-ins.

**Workaround**

Plug-ins: Enable JUCE_FORCE_LEGACY_PARAMETER_AUTOMATION_TYPE in the
juce_audio_plugin_client module config page in the Projucer.

Hosts: Use AudioPluginInstance::getDefaultNumParameterSteps as the number of
steps for all parameters.

**Rationale**

The old system for presenting plug-in parameters to a host as either continuous
or discrete is inconsistent between plug-in types and lacks sufficient
flexibility. This change harmonises the behaviour and allows individual
parameters to be marked as continuous or discrete. If AudioUnit and AudioUnit
v3 parameters are not marked as high precision then hosts like Logic Pro only
offer a limited number of parameter values, which again produces different
behaviour for different plug-in types.


## Change

A new FrameRateType fps23976 has been added to AudioPlayHead,

**Possible Issues**

Previously JUCE would report the FrameRateType fps24 for both 24 and 23.976
fps. If your code uses switch statements (or similar) to handle all possible
frame rate types, then this change may cause it to fall through.

**Workaround**

Add fps23976 to your switch statement and handle it appropriately.

**Rationale**

JUCE should be able to handle all popular frame rate codes but was missing
support for 23.976.


## Change

The String (bool) constructor and operator<< (String&, bool) have been
explicitly deleted.

**Possible Issues**

Previous code which relied on an implicit bool to int type conversion to
produce a String will not compile.

**Workaround**

Cast your bool to an integer to generate a string representation of it.

**Rationale**

Letting things implicitly convert to bool to produce a String opens the door to
all kinds of nasty type conversion edge cases. Furthermore, before this change,
MacOS would automatically convert bools to ints but this wouldn't occur on
different platform. Now the behaviour is consistent across all operating
systems supported by JUCE.


## Change

The writeAsJSON virtual method of the DynamicObject class requires an
additional parameter, maximumDecimalPlaces, to specify the maximum precision of
floating point numbers.

**Possible Issues**

Classes which inherit from DynamicObject and override this method will need to
update their method signature.

**Workaround**

Your custom DynamicObject class can choose to ignore the additional parameter
if you don't wish to support this behaviour.

**Rationale**

When serialising the results of calculations to JSON the rounding of floating
point numbers can result in numbers with 17 significant figures where only a
few are required. This change to DynamicObject is required to support
truncating those numbers.


# Version 5.1.0

## Change

The JUCE_COMPILER_SUPPORTS_LAMBDAS preprocessor macro has been removed.

**Possible Issues**

If your project is using JUCE_COMPILER_SUPPORTS_LAMBDAS in your source code
then it will likely evaluate to "false" and you could end up unnecessarily
using code paths which avoid lambda functions.

**Workaround**

Remove the usage of JUCE_COMPILER_SUPPORTS_LAMBDAS from your code.

**Rationale**

Lambda functions are now available on all platforms that JUCE supports.


## Change

The option to set the C++ language standard is now located in the project
settings instead of the build configuration settings.

**Possible Issues**

Projects that had a specific version of the C++ language standard set for
exporter build configurations will instead use the default (C++11) when
re-saving with the new Projucer.

**Workaround**

Change the "C++ Language Standard" setting in the main project settings to the
required version - the Projucer will add this value to the exported project as
a compiler flag when saving exporters.

**Rationale**

Having a different C++ language standard option for each build configuration
was unnecessary and was not fully implemented for all exporters. Changing it to
a per-project settings means that the preference will propagate to all
exporters and only needs to be set in one place.


## Change

PopupMenus now scale according to the AffineTransform and scaling factor of
their target components.

**Possible Issues**

Developers who have manually scaled their PopupMenus to fit the scaling factor
of the parent UI will now have the scaling applied two times in a row.

**Workaround**

1. Do not apply your own manual scaling to make your popups match the UI
   scaling

or

2. Override the Look&Feel method
   PopupMenu::LookAndFeelMethods::shouldPopupMenuScaleWithTargetComponent and
   return false. See
   https://github.com/juce-framework/JUCE/blob/c288c94c2914af20f36c03ca9c5401fcb555e4e9/modules/juce_gui_basics/menus/juce_PopupMenu.h#725

**Rationale**

Previously, PopupMenus would not scale if the GUI of the target component (or
any of its parents) were scaled. The only way to scale PopupMenus was via the
global scaling factor. This had several drawbacks as the global scaling factor
would scale everything. This was especially problematic in plug-in editors.


## Change

Removed the setSecurityFlags() method from the Windows implementation of
WebInputStream as it disabled HTTPS security features.

**Possible Issues**

Any code previously relying on connections to insecure webpages succeeding will
no longer work.

**Workaround**

Check network connectivity on Windows and re-write any code that relied on
insecure connections.

**Rationale**

The previous behaviour resulted in network connections on Windows having all
the HTTPS security features disabled, exposing users to network attacks. HTTPS
connections on Windows are now secure and will fail when connecting to an
insecure web address.


## Change

Pointer arithmetic on a pointer will have the same result regardless if it is
wrapped in JUCE's Atomic class or not.

**Possible Issues**

Any code using pointer arithmetic on Atomic<T*> will now have a different
result leading to undefined behaviour or crashes.

**Workaround**

Re-write your code in a way that it does not depend on your pointer being
wrapped in JUCE's Atomic or not. See rationale.

**Rationale**

Before this change, pointer arithmetic with JUCE's Atomic type would yield
confusing results. For example, the following code would assert before this
change:

int* a; Atomic<int*> b;

jassert (++a == ++b);

Pointer a in the above code would be advanced by sizeof(int) whereas the JUCE's
Atomic always advances it's underlying pointer by a single byte. The same is
true for operator+=/operator-= and operator--. The difference in behaviour is
confusing and unintuitive. Furthermore, this aligns JUCE's Atomic type with
std::atomic.


# Version 4.3.1

## Change

JUCE has changed the way native VST3/AudioUnit parameter ids are calculated.

**Possible Issues**

DAW projects with automation data written by an AudioUnit or VST3 plug-in built
with pre JUCE 4.3.1 versions will load incorrectly when opened by an AudioUnit
or VST3 built with JUCE versions 4.3.1 and later. Plug-ins using
JUCE_FORCE_USE_LEGACY_PARAM_IDS are not affected.

**Workaround**

Disable JUCE_USE_STUDIO_ONE_COMPATIBLE_PARAMETERS in the
juce_audio_plugin_client module config page in the Projucer. For new plug-ins,
be sure to use the default value for this property.

**Rationale**

JUCE needs to convert between its own JUCE parameter id format (strings) to the
native parameter id formats of the various plug-in backends. For VST3 and
AudioUnits, JUCE uses a hash function to generate a numeric id. However, some
VST3/AudioUnit hosts (specifically Studio One) have a bug that ignore any
parameters that have a negative parameter id. Therefore, the hash function for
VST3/AudioUnits needed to be changed to only return positive-valued hashes.


# Version 4.3.0

## Change

A revised multi-bus API was released which supersedes the previously flawed
multi-bus API - JUCE versions 4.0.0 - 4.2.4 (inclusive).

**Possible Issues**

If you have developed a plug-in with JUCE versions 4.0.0 - 4.2.4 (inclusive),
then you will need to update your plug-in to the new multi-bus API. Pre JUCE
4.0.0 plug-ins are not affected apart from other breaking changes listed in
this document.

**Workaround**

None.

**Rationale**

A flawed multi-bus API was introduced with JUCE versions 4.0.0 up until version
4.2.4 (inclusive) which was not API compatible with pre JUCE 4 plug-ins. JUCE
4.3.0 releases a revised multi-bus API which restores pre JUCE 4 API
compatibility. However, the new multi-bus API is not compatible with the flawed
multi-bus API (JUCE version 4.0.0 - 4.2.4).


## Change

JUCE now generates the AAX plug-in bus layout configuration id independent from
the position as it appears in the Projucer’s legacy "Channel layout
configuration" field.

**Possible Issues**

ProTools projects generated with a < 4.3.0 JUCE versions of your plug-in, may
load the incorrect bus configuration when upgrading your plug-in to >= 4.3.0
versions of JUCE.

**Workaround**

Implement AudioProcessor’s getAAXPluginIDForMainBusConfig callback to manually
override which AAX plug-in id is associated to a specific bus layout of your
plug-in. This workaround is only necessary if you have released your plug-in
built with a version previous to JUCE 4.3.0.

**Rationale**

The new multi-bus API offers more features, flexibility and accuracy in
specifying bus layouts which cannot be expressed by the Projucer’s legacy
"Channel layout configuration" field. The native plug-in format backends use
the new multi-bus callback APIs to negotiate channel layouts with the host -
including the AAX plug-in ids assigned to specific bus layouts. With the
callback API, there is no notion of an order in which the channel
configurations appear - as was the case with the legacy "Channel layout
configuration" field - and therefore cannot be used to generate the AAX plug-in
id. To remain backward compatible to pre JUCE 4.0.0 plug-ins, JUCE does
transparently convert the legacy "Channel layout configuration" field to the
new callback based multi-bus API, but this does not take the order into account
in which the channel configurations appear in the legacy "Channel layout
configuration" field.


# Version 4.2.1

## Change

JUCE now uses the paramID property used in AudioProcessorParameterWithID to
uniquely identify parameters to the host.

**Possible Issues**

DAW projects with automation data written by an audio plug-in built with pre
JUCE 4.2.1 will load incorrectly when opened by an audio plug-in built with
JUCE 4.2.1 and later.

**Workaround**

Enable JUCE_FORCE_USE_LEGACY_PARAM_IDS in the juce_audio_plugin_client module config
page in the Projucer. For new plug-ins, be sure to disable this property.

**Rationale**

Each parameter of the AudioProcessor has an id associated so that the plug-in’s
host can uniquely identify parameters. The id has a different data-type for
different plug-in types (for example VST uses integers, AAX uses string
identifiers). Before 4.2.1, JUCE generated the parameter id by using the index
of the parameter, i.e. the first parameter had id zero, the second parameter
had id one, etc. This caused problems for certain plug-in types where JUCE
needs to add internal parameters to the plug-in (for example VST3 requires the
bypass control to be a parameter - so JUCE automatically creates this parameter
for you in the VST3 backend). This causes subtle problems if a parameter is
added to an update of an already published plug-in. The new parameter’s id
would be identical to the id of the bypass parameter in old versions of your
plug-in, causing seemingly random plug-in bypass behaviour when user’s upgrade
their plug-in.

Most plug-in backends differentiate between a parameter’s id an index, so this
distinction was adopted starting with JUCE 4.2.1 by deriving the parameter’s
unique id from the paramID property of AudioProcessorParameterWithID class.
