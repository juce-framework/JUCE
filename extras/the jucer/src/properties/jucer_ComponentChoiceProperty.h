/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_COMPONENTCHOICEPROPERTY_JUCEHEADER__
#define __JUCER_COMPONENTCHOICEPROPERTY_JUCEHEADER__


//==============================================================================
/**
*/
template <class ComponentType>
class ComponentChoiceProperty  : public ChoicePropertyComponent,
                                 private ChangeListener
{
public:
    ComponentChoiceProperty (const String& name,
                             ComponentType* component_,
                             JucerDocument& document_)
        : ChoicePropertyComponent (name),
          component (component_),
          document (document_)
    {
        document.addChangeListener (this);
    }

    ~ComponentChoiceProperty()
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

protected:
    ComponentType* component;
    JucerDocument& document;
};


#endif   // __JUCER_COMPONENTCHOICEPROPERTY_JUCEHEADER__
