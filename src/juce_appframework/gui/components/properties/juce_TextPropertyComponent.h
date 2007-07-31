/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__

#include "juce_PropertyComponent.h"
#include "../controls/juce_Label.h"


//==============================================================================
/**
    A PropertyComponent that shows its value as editable text.

    @see PropertyComponent
*/
class JUCE_API  TextPropertyComponent  : public PropertyComponent
{
public:
    //==============================================================================
    /** Creates a text property component.

        The maxNumChars is used to set the length of string allowable, and isMultiLine
        sets whether the text editor allows carriage returns.

        @see TextEditor
    */
    TextPropertyComponent (const String& propertyName,
                           const int maxNumChars,
                           const bool isMultiLine);

    /** Destructor. */
    ~TextPropertyComponent();

    //==============================================================================
    /** Called when the user edits the text.

        Your subclass must use this callback to change the value of whatever item
        this property component represents.
    */
    virtual void setText (const String& newText) = 0;

    /** Returns the text that should be shown in the text editor.
    */
    virtual const String getText() const = 0;


    //==============================================================================
    /** @internal */
    void refresh();
    /** @internal */
    void textWasEdited();

    juce_UseDebuggingNewOperator

private:
    Label* textEditor;
};


#endif   // __JUCE_TEXTPROPERTYCOMPONENT_JUCEHEADER__
