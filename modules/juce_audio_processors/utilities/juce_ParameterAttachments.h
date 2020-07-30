/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
    /** Listens to a parameter and calls the the provided function in response to
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

} // namespace juce
