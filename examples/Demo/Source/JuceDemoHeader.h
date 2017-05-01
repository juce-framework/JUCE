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

#pragma once

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
