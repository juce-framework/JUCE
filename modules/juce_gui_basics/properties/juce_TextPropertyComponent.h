/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A PropertyComponent that shows its value as editable text.

    @see PropertyComponent
*/
class JUCE_API  TextPropertyComponent  : public PropertyComponent
{
protected:
    //==============================================================================
    /** Creates a text property component.

        @param propertyName  The name of the property
        @param maxNumChars   If not zero, then this specifies the maximum allowable length of
                             the string. If zero, then the string will have no length limit.
        @param isMultiLine   isMultiLine sets whether the text editor allows carriage returns.

        @see TextEditor
    */
    TextPropertyComponent (const String& propertyName,
                           int maxNumChars,
                           bool isMultiLine);

public:
    /** Creates a text property component.

        @param valueToControl The Value that is controlled by the TextPropertyComponent
        @param propertyName   The name of the property
        @param maxNumChars    If not zero, then this specifies the maximum allowable length of
                              the string. If zero, then the string will have no length limit.
        @param isMultiLine    isMultiLine sets whether the text editor allows carriage returns.

        @see TextEditor
    */
    TextPropertyComponent (const Value& valueToControl,
                           const String& propertyName,
                           int maxNumChars,
                           bool isMultiLine);

    /** Destructor. */
    ~TextPropertyComponent();

    //==============================================================================
    /** Called when the user edits the text.

        Your subclass must use this callback to change the value of whatever item
        this property component represents.
    */
    virtual void setText (const String& newText);

    /** Returns the text that should be shown in the text editor. */
    virtual String getText() const;

    /** Returns the text that should be shown in the text editor as a Value object. */
    Value& getValue() const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x100e401,    /**< The colour to fill the background of the text area. */
        textColourId                = 0x100e402,    /**< The colour to use for the editable text. */
        outlineColourId             = 0x100e403,    /**< The colour to use to draw an outline around the text area. */
    };

    void colourChanged() override;

    //==============================================================================
    class JUCE_API Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() {}

        /** Called when text has finished being entered (i.e. not per keypress) has changed. */
        virtual void textPropertyComponentChanged (TextPropertyComponent*) = 0;
    };

    /** Registers a listener to receive events when this button's state changes.
        If the listener is already registered, this will not register it again.
        @see removeListener
    */
    void addListener (Listener* newListener);

    /** Removes a previously-registered button listener
        @see addListener
    */
    void removeListener (Listener* listener);

    //==============================================================================
    /** Sets whether the text property component can have files dropped onto it by an external application.

        The default setting for this is true but you may want to disable this behaviour if you derive
        from this class and want your subclass to respond to the file drag.
    */
    void setInterestedInFileDrag (bool isInterested);

    //==============================================================================
    /** @internal */
    void refresh() override;
    /** @internal */
    virtual void textWasEdited();

private:
    class LabelComp;
    friend class LabelComp;

    ScopedPointer<LabelComp> textEditor;
    ListenerList<Listener> listenerList;

    void callListeners();
    void createEditor (int maxNumChars, bool isMultiLine);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextPropertyComponent)
};

#ifndef DOXYGEN
 /** This typedef is just for compatibility with old code and VC6 - newer code should use TextPropertyComponent::Listener instead. */
 typedef TextPropertyComponent::Listener TextPropertyComponentListener;
#endif

} // namespace juce
