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

#ifndef JUCER_COMPONENTCHOICEPROPERTY_H_INCLUDED
#define JUCER_COMPONENTCHOICEPROPERTY_H_INCLUDED


template <class ComponentType>
class ComponentChoiceProperty  : public ChoicePropertyComponent,
                                 private ChangeListener
{
public:
    ComponentChoiceProperty (const String& name,
                             ComponentType* comp,
                             JucerDocument& doc)
        : ChoicePropertyComponent (name),
          component (comp),
          document (doc)
    {
        document.addChangeListener (this);
    }

    ~ComponentChoiceProperty()
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

protected:
    ComponentType* component;
    JucerDocument& document;
};


#endif   // JUCER_COMPONENTCHOICEPROPERTY_H_INCLUDED
