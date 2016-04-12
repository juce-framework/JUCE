/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_TEXTPROPERTYCOMPONENT_H_INCLUDED
#define JUCE_TEXTPROPERTYCOMPONENT_H_INCLUDED


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

        The maxNumChars is used to set the length of string allowable, and isMultiLine
        sets whether the text editor allows carriage returns.

        @see TextEditor
    */
    TextPropertyComponent (const String& propertyName,
                           int maxNumChars,
                           bool isMultiLine);

public:
    /** Creates a text property component.

        The maxNumChars is used to set the length of string allowable, and isMultiLine
        sets whether the text editor allows carriage returns.

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

#endif   // JUCE_TEXTPROPERTYCOMPONENT_H_INCLUDED
