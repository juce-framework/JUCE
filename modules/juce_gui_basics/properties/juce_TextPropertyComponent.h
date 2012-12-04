/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__

#include "juce_PropertyComponent.h"
#include "../widgets/juce_Label.h"


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

    //==============================================================================
    /** @internal */
    void refresh();

private:
    ScopedPointer<Label> textEditor;

    class LabelComp;
    friend class LabelComp;

    void textWasEdited();
    void createEditor (int maxNumChars, bool isMultiLine);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextPropertyComponent)
};


#endif   // __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__
