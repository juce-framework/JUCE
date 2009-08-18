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

#ifndef __JUCER_COMPONENTBOOLEANPROPERTY_JUCEHEADER__
#define __JUCER_COMPONENTBOOLEANPROPERTY_JUCEHEADER__


//==============================================================================
/**
*/
template <class ComponentType>
class ComponentBooleanProperty  : public BooleanPropertyComponent,
                                  private ChangeListener
{
public:
    ComponentBooleanProperty (const String& name,
                              const String& onText,
                              const String& offText,
                              ComponentType* component_,
                              JucerDocument& document_)
        : BooleanPropertyComponent (name, onText, offText),
          component (component_),
          document (document_)
    {
        document.addChangeListener (this);
    }

    ~ComponentBooleanProperty()
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


#endif   // __JUCER_COMPONENTBOOLEANPROPERTY_JUCEHEADER__
