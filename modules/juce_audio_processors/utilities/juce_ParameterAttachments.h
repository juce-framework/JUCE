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

/** Used to implement 'attachments' or 'controllers' that link a plug-in
    parameter to a UI element.

    To implement a new attachment type, create a new class which includes an
    instance of this class as a data member. Your class should pass a function
    to the constructor of the ParameterAttachment, which will then be called on
    the message thread when the parameter changes. You can use this function to
    update the state of the UI control. Your class should also register as a
    listener of the UI control and respond to respond to changes in the UI element
    by calling either setValueAsCompleteGesture or beginGesture,
    setValueAsPartOfGesture and endGesture.

    Make sure to call `sendInitialUpdate` at the end of your new attachment's
    constructor, so that the UI immediately reflects the state of the parameter.

    @tags{Audio}
*/
class ParameterAttachment   : private AudioProcessorParameter::Listener,
                              private AsyncUpdater
{
public:
    /** Listens to a parameter and calls the provided function in response to
        parameter changes. If an undoManager is supplied `beginNewTransaction` will
        be called on it whenever the UI requests a parameter change via this attachment.

        @param parameter                  The parameter to which this attachment will listen
        @param parameterChangedCallback   The function that will be called on the message thread in response
                                          to parameter changes
        @param undoManager                The UndoManager that will be used to begin transactions when the UI
                                          requests a parameter change.
    */
    ParameterAttachment (RangedAudioParameter& parameter,
                         std::function<void (float)> parameterChangedCallback,
                         UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~ParameterAttachment() override;

    /** Calls the parameterChangedCallback function that was registered in
        the constructor, making the UI reflect the current parameter state.

        This function should be called after doing any necessary setup on
        the UI control that is being managed (e.g. adding ComboBox entries,
        making buttons toggle-able).
    */
    void sendInitialUpdate();

    /** Triggers a full gesture message on the managed parameter.

        Call this in the listener callback of the UI control in response
        to a one-off change in the UI like a button-press.
    */
    void setValueAsCompleteGesture (float newDenormalisedValue);

    /** Begins a gesture on the managed parameter.

        Call this when the UI is about to begin a continuous interaction,
        like when the mouse button is pressed on a slider.
    */
    void beginGesture();

    /** Updates the parameter value during a gesture.

        Call this during a continuous interaction, like a slider value
        changed callback.
    */
    void setValueAsPartOfGesture (float newDenormalisedValue);

    /** Ends a gesture on the managed parameter.

        Call this when the UI has finished a continuous interaction,
        like when the mouse button is released on a slider.
    */
    void endGesture();

private:
    float normalise (float f) const   { return parameter.convertTo0to1 (f); }

    template <typename Callback>
    void callIfParameterValueChanged (float newDenormalisedValue, Callback&& callback);

    void parameterValueChanged (int, float) override;
    void parameterGestureChanged (int, bool) override {}
    void handleAsyncUpdate() override;

    RangedAudioParameter& parameter;
    std::atomic<float> lastValue { 0.0f };
    UndoManager* undoManager = nullptr;
    std::function<void (float)> setValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParameterAttachment)
};

//==============================================================================
/** An object of this class maintains a connection between a Slider and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making
    it easy to connect a slider to a parameter. When this object is deleted, the
    connection is broken. Make sure that your parameter and Slider are not
    deleted before this object!

    @tags{Audio}
*/
class SliderParameterAttachment   : private Slider::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a Slider.

        @param parameter     The parameter to use
        @param slider        The Slider to use
        @param undoManager   An optional UndoManager
    */
    SliderParameterAttachment (RangedAudioParameter& parameter, Slider& slider,
                               UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~SliderParameterAttachment() override;

    /** Call this after setting up your slider in the case where you need to do
        extra setup after constructing this attachment.
    */
    void sendInitialUpdate();

private:
    void setValue (float newValue);
    void sliderValueChanged (Slider*) override;

    void sliderDragStarted (Slider*) override { attachment.beginGesture(); }
    void sliderDragEnded   (Slider*) override { attachment.endGesture(); }

    Slider& slider;
    ParameterAttachment attachment;
    bool ignoreCallbacks = false;
};

//==============================================================================
/** An object of this class maintains a connection between a ComboBox and a
    plug-in parameter.

    ComboBox items will be spaced linearly across the range of the parameter. For
    example if the range is specified by NormalisableRange<float> (-0.5f, 0.5f, 0.5f)
    and you add three items then the first will be mapped to a value of -0.5, the
    second to 0, and the third to 0.5.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a combo box to a parameter. When this object is deleted, the
    connection is broken. Make sure that your parameter and ComboBox are not deleted
    before this object!

    @tags{Audio}
*/
class ComboBoxParameterAttachment   : private ComboBox::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a ComboBox.

        @param parameter     The parameter to use
        @param combo         The ComboBox to use
        @param undoManager   An optional UndoManager
    */
    ComboBoxParameterAttachment (RangedAudioParameter& parameter, ComboBox& combo,
                                 UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~ComboBoxParameterAttachment() override;

    /** Call this after setting up your combo box in the case where you need to do
        extra setup after constructing this attachment.
    */
    void sendInitialUpdate();

private:
    void setValue (float newValue);
    void comboBoxChanged (ComboBox*) override;

    ComboBox& comboBox;
    RangedAudioParameter& storedParameter;
    ParameterAttachment attachment;
    bool ignoreCallbacks = false;
};

//==============================================================================
/** An object of this class maintains a connection between a Button and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a button to a parameter. When this object is deleted, the
    connection is broken. Make sure that your parameter and Button are not deleted
    before this object!

    @tags{Audio}
*/
class ButtonParameterAttachment   : private Button::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a Button.

        @param parameter     The parameter to use
        @param button        The Button to use
        @param undoManager   An optional UndoManager
    */
    ButtonParameterAttachment (RangedAudioParameter& parameter, Button& button,
                               UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~ButtonParameterAttachment() override;

    /** Call this after setting up your button in the case where you need to do
        extra setup after constructing this attachment.
    */
    void sendInitialUpdate();

private:
    void setValue (float newValue);
    void buttonClicked (Button*) override;

    Button& button;
    ParameterAttachment attachment;
    bool ignoreCallbacks = false;
};

#if JUCE_WEB_BROWSER || DOXYGEN
//==============================================================================
/**
    An object of this class maintains a connection between a WebSliderRelay and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a WebSliderRelay to a parameter. When this object is deleted,
    the connection is broken. Make sure that your parameter and WebSliderRelay are
    not deleted before this object!

    @tags{Audio}
*/
class WebSliderParameterAttachment   : private WebSliderRelay::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a WebSliderRelay.

        @param parameterIn   The parameter to use
        @param sliderStateIn The WebSliderRelay to use
        @param undoManager   An optional UndoManager
    */
    WebSliderParameterAttachment (RangedAudioParameter& parameterIn,
                                  WebSliderRelay& sliderStateIn,
                                  UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~WebSliderParameterAttachment() override;

    /** Call this after setting up your slider in the case where you need to do
        extra setup after constructing this attachment.
    */
    void sendInitialUpdate();

private:
    void setValue (float newValue);

    void sliderValueChanged (WebSliderRelay*) override;

    void sliderDragStarted (WebSliderRelay*) override      { attachment.beginGesture(); }
    void sliderDragEnded (WebSliderRelay*) override        { attachment.endGesture(); }
    void initialUpdateRequested (WebSliderRelay*) override { sendInitialUpdate(); }

    WebSliderRelay& sliderState;
    RangedAudioParameter& parameter;
    ParameterAttachment attachment;
    bool ignoreCallbacks = false;
};

//==============================================================================
/**
    An object of this class maintains a connection between a WebToggleButtonRelay and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a WebToggleButtonRelay to a parameter. When this object is deleted,
    the connection is broken. Make sure that your parameter and WebToggleButtonRelay are
    not deleted before this object!

    @tags{Audio}
*/
class WebToggleButtonParameterAttachment  : private WebToggleButtonRelay::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a WebToggleButtonRelay.

        @param parameterIn   The parameter to use
        @param button        The WebToggleButtonRelay to use
        @param undoManager   An optional UndoManager
    */
    WebToggleButtonParameterAttachment (RangedAudioParameter& parameterIn,
                                        WebToggleButtonRelay& button,
                                        UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~WebToggleButtonParameterAttachment() override;

    /** Call this after setting up your button in the case where you need to do
        extra setup after constructing this attachment.
    */
    void sendInitialUpdate();

private:
    void setValue (float newValue);

    void toggleStateChanged (bool newValue) override;
    void initialUpdateRequested() override;

    WebToggleButtonRelay& relay;
    RangedAudioParameter& parameter;
    ParameterAttachment attachment;
    bool ignoreCallbacks = false;
};

//==============================================================================
/**
    An object of this class maintains a connection between a WebComboBoxRelay and a
    plug-in parameter.

    During the lifetime of this object it keeps the two things in sync, making it
    easy to connect a WebComboBoxRelay to a parameter. When this object is deleted,
    the connection is broken. Make sure that your parameter and WebComboBoxRelay are
    not deleted before this object!

    @tags{Audio}
*/
class WebComboBoxParameterAttachment   : private WebComboBoxRelay::Listener
{
public:
    /** Creates a connection between a plug-in parameter and a WebComboBoxRelay.

        @param parameterIn   The parameter to use
        @param combo         The WebComboBoxRelay to use
        @param undoManager   An optional UndoManager
    */
    WebComboBoxParameterAttachment (RangedAudioParameter& parameterIn, WebComboBoxRelay& combo,
                                    UndoManager* undoManager = nullptr);

    /** Destructor. */
    ~WebComboBoxParameterAttachment() override;

    /** Call this after setting up your combo box in the case where you need to do
        extra setup after constructing this attachment.
    */
    void sendInitialUpdate();

private:
    void setValue (float newValue);

    void valueChanged (float newValue) override;
    void initialUpdateRequested() override;

    WebComboBoxRelay& relay;
    RangedAudioParameter& parameter;
    ParameterAttachment attachment;
    bool ignoreCallbacks = false;
};

#endif

} // namespace juce
