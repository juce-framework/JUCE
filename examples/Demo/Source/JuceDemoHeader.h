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

#ifndef __JUCEDEMOHEADER_H_EE664D1A__
#define __JUCEDEMOHEADER_H_EE664D1A__

#include "../JuceLibraryCode/JuceHeader.h"
#include "MainWindow.h"
#include "DemoUtilities.h"


//==============================================================================
/** Static subclasses of this class are created in each of the demo modules, to
    register each of the demo types.
*/
class JuceDemoTypeBase
{
public:
    JuceDemoTypeBase (const String& demoName);
    virtual ~JuceDemoTypeBase();

    virtual Component* createComponent() = 0;

    const String name;

    static Array<JuceDemoTypeBase*>& getDemoTypeList();

private:
    JUCE_DECLARE_NON_COPYABLE (JuceDemoTypeBase)
};

//==============================================================================
/** A templated subclass of JuceDemoTypeBase to make it easy for our demos
    to declare themselves.
*/
template <class DemoType>
class JuceDemoType      : public JuceDemoTypeBase
{
public:
    JuceDemoType (const String& demoName)  : JuceDemoTypeBase (demoName)
    {}

    Component* createComponent()    { return new DemoType(); }

private:
    JUCE_DECLARE_NON_COPYABLE (JuceDemoType)
};


#endif  // __JUCEDEMOHEADER_H_EE664D1A__
