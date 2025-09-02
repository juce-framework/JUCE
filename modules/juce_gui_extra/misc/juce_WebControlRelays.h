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

/** Helper class that relays audio parameter information to an object inside a WebBrowserComponent.

    In order to create a relay you need to specify an identifier for the relayed state. This will
    result in a Javascript object becoming available inside the WebBrowserComponent under the
    provided identifier.

    Pass the relay object to WebBrowserComponent::Options::withOptionsFrom() to associate it with
    a WebBrowserComponent instance.

    You can then use a WebSliderParameterAttachment as you would a SliderAttachment, to attach the
    relay to a RangedAudioParameter. This will synchronise the state and events of the Javascript
    object with the audio parameter at all times.

    @code
    // Add a relay to your AudioProcessorEditor members
    WebSliderRelay cutoffSliderRelay { "cutoffSlider" };
    WebBrowserComponent webComponent { WebBrowserComponent::Options{}::withOptionsFrom (cutoffSliderRelay) };
    @endcode

    @code
    // In your Javascript GUI code you obtain an object from the framework
    import * as Juce from "juce-framework-frontend";
    const sliderState = Juce.getSliderState("cutoffSlider");
    @endcode

    @see WebSliderParameterAttachment

    @tags{GUI}
*/
class JUCE_API  WebSliderRelay : public OptionsBuilder<WebBrowserComponent::Options>,
                                 private WebViewLifetimeListener
{
public:
    /** Creating a relay will ensure that a Javascript object under the provided name will be
        available in the specified WebBrowserComponent's context. Use the frontend framework's
        getSliderState function with the same name to get a hold of this object.
    */
    WebSliderRelay (StringRef nameIn);

    //==============================================================================
    /** @internal */
    struct Listener : public SliderListener<WebSliderRelay>
    {
        virtual void initialUpdateRequested (WebSliderRelay*) = 0;
    };

    /** @internal */
    void setValue (float newValue);

    /** @internal */
    float getValue() const;

    /** @internal */
    void addListener (Listener* l);

    /** @internal */
    void removeListener (Listener* l);

    /** @internal */
    WebBrowserComponent::Options buildOptions (const WebBrowserComponent::Options& initialOptions) override;

    /** @internal */
    void emitEvent (const var& payload);

private:
    void handleEvent (const var& event);
    void webViewConstructed (WebBrowserComponent*) override;
    void webViewDestructed (WebBrowserComponent*) override;

    WebBrowserComponent* browser = nullptr;
    String name;
    float value{};
    Identifier eventId { "__juce__slider" + name };
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE (WebSliderRelay)
    JUCE_DECLARE_NON_MOVEABLE (WebSliderRelay)
};

/** Helper class that relays audio parameter information to an object inside a WebBrowserComponent.

    In order to create a relay you need to specify an identifier for the relayed state. This will
    result in a Javascript object becoming available inside the WebBrowserComponent under the
    provided identifier.

    Pass the relay object to WebBrowserComponent::Options::withOptionsFrom() to associate it with
    a WebBrowserComponent instance.

    You can then use a WebToggleButtonParameterAttachment as you would a ButtonParameterAttachment,
    to attach the relay to a RangedAudioParameter. This will synchronise the state and events of
    the Javascript object with the audio parameter at all times.

    @code
    // Add a relay to your AudioProcessorEditor members
    WebToggleButtonRelay muteToggleRelay { "muteToggle" };
    WebBrowserComponent webComponent { WebBrowserComponent::Options{}::withOptionsFrom (muteToggleRelay) };
    @endcode

    @code
    // In your Javascript GUI code you obtain an object from the framework
    import * as Juce from "juce-framework-frontend";
    const checkboxState = Juce.getToggleState("muteToggle");
    @endcode

    @see WebToggleButtonParameterAttachment

    @tags{GUI}
*/
class JUCE_API  WebToggleButtonRelay  : public OptionsBuilder<WebBrowserComponent::Options>,
                                        private WebViewLifetimeListener
{
public:
    /** Creating a relay will ensure that a Javascript object under the provided name will be
        available in the specified WebBrowserComponent's context. Use the frontend framework's
        getToggleState function with the same name to get a hold of this object.
    */
    WebToggleButtonRelay (StringRef nameIn);

    //==============================================================================
    /** @internal */
    struct Listener
    {
        virtual ~Listener()                          = default;
        virtual void toggleStateChanged (bool)       = 0;
        virtual void initialUpdateRequested()        = 0;
    };

    /** @internal */
    void setToggleState (bool newState);

    /** @internal */
    void addListener (Listener* l);

    /** @internal */
    void removeListener (Listener* l);

    /** @internal */
    WebBrowserComponent::Options buildOptions (const WebBrowserComponent::Options& initialOptions) override;

    /** @internal */
    void emitEvent (const var& payload);

private:
    void handleEvent (const var& event);
    void webViewConstructed (WebBrowserComponent*) override;
    void webViewDestructed (WebBrowserComponent*) override;

    WebBrowserComponent* browser = nullptr;
    String name;
    Identifier eventId { "__juce__toggle" + name };
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE (WebToggleButtonRelay)
    JUCE_DECLARE_NON_MOVEABLE (WebToggleButtonRelay)
};

/** Helper class that relays audio parameter information to an object inside a WebBrowserComponent.

    In order to create a relay you need to specify an identifier for the relayed state. This will
    result in a Javascript object becoming available inside the WebBrowserComponent under the
    provided identifier.

    Pass the relay object to WebBrowserComponent::Options::withOptionsFrom() to associate it with
    a WebBrowserComponent instance.

    You can then use a WebComboBoxParameterAttachment as you would a ComboBoxParameterAttachment,
    to attach the relay to a RangedAudioParameter. This will synchronise the state and events of
    the Javascript object with the audio parameter at all times.

    @code
    // Add a relay to your AudioProcessorEditor members
    WebComboBoxRelay filterTypeComboRelay { "filterTypeCombo" };
    WebBrowserComponent webComponent { WebBrowserComponent::Options{}::withOptionsFrom (filterTypeComboRelay) };
    @endcode

    @code
    // In your Javascript GUI code you obtain an object from the framework
    import * as Juce from "juce-framework-frontend";
    const comboBoxState = Juce.getComboBoxState("filterTypeCombo");
    @endcode

    @see WebComboBoxParameterAttachment

    @tags{GUI}
*/
class JUCE_API  WebComboBoxRelay  : public OptionsBuilder<WebBrowserComponent::Options>,
                                    private WebViewLifetimeListener
{
public:
    /** Creating a relay will ensure that a Javascript object under the provided name will be
        available in the specified WebBrowserComponent's context. Use the frontend framework's
        getComboBoxState function with the same name to get a hold of this object.
    */
    WebComboBoxRelay (StringRef nameIn);

    //==============================================================================
    /** @internal */
    struct Listener
    {
        virtual ~Listener()                          = default;
        virtual void valueChanged (float)            = 0;
        virtual void initialUpdateRequested()        = 0;
    };

    /** @internal */
    void setValue (float newValue);

    /** @internal */
    void addListener (Listener* l);

    /** @internal */
    void removeListener (Listener* l);

    /** @internal */
    WebBrowserComponent::Options buildOptions (const WebBrowserComponent::Options& initialOptions) override;

    /** @internal */
    void emitEvent (const var& payload);

private:
    void handleEvent (const var& event);
    void webViewConstructed (WebBrowserComponent*) override;
    void webViewDestructed (WebBrowserComponent*) override;

    WebBrowserComponent* browser = nullptr;
    String name;
    Identifier eventId { "__juce__comboBox" + name };
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE (WebComboBoxRelay)
    JUCE_DECLARE_NON_MOVEABLE (WebComboBoxRelay)
};

#endif

}
