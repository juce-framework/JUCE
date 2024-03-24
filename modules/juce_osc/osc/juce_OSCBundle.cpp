/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

OSCBundle::OSCBundle()
{
}

OSCBundle::OSCBundle (OSCTimeTag t)  : timeTag (t)
{
}

// Note: The class invariant of OSCBundle::Element is that
// at least one of the pointers bundle and message is nullptr
// and the other one always points to a valid object.

OSCBundle::Element::Element (OSCMessage m)
    : message (new OSCMessage (m)), bundle (nullptr)
{
}

OSCBundle::Element::Element (OSCBundle b)
    : message (nullptr), bundle (new OSCBundle (b))
{
}

//==============================================================================
OSCBundle::Element::Element (const Element& other)
{
    if (this != &other)
    {
        message = nullptr;
        bundle = nullptr;

        if (other.isMessage())
            message.reset (new OSCMessage (other.getMessage()));
        else
            bundle.reset (new OSCBundle (other.getBundle()));
    }
}

//==============================================================================
OSCBundle::Element::~Element()
{
    bundle = nullptr;
    message = nullptr;
}

//==============================================================================
bool OSCBundle::Element::isMessage() const noexcept
{
    return message != nullptr;
}

bool OSCBundle::Element::isBundle() const noexcept
{
    return bundle != nullptr;
}

//==============================================================================
const OSCMessage& OSCBundle::Element::getMessage() const
{
    if (message == nullptr)
    {
        // This element is not a bundle! You must check this first before accessing.
        jassertfalse;
        throw OSCInternalError ("Access error in OSC bundle element.");
    }

    return *message;
}

//==============================================================================
const OSCBundle& OSCBundle::Element::getBundle() const
{
    if (bundle == nullptr)
    {
        // This element is not a bundle! You must check this first before accessing.
        jassertfalse;
        throw OSCInternalError ("Access error in OSC bundle element.");
    }

    return *bundle;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class OSCBundleTests final : public UnitTest
{
public:
    OSCBundleTests()
        : UnitTest ("OSCBundle class", UnitTestCategories::osc)
    {}

    void runTest() override
    {
        beginTest ("Construction");
        {
            OSCBundle bundle;
            expect (bundle.getTimeTag().isImmediately());
        }

        beginTest ("Construction with time tag");
        {
            Time in100Seconds = (Time (Time::currentTimeMillis()) + RelativeTime (100.0));
            OSCBundle bundle (in100Seconds);
            expect (! bundle.getTimeTag().isImmediately());
            expect (bundle.getTimeTag().toTime() == in100Seconds);
        }

        beginTest ("Usage when containing messages");
        {
            OSCBundle testBundle = generateTestBundle();
            expectBundleEqualsTestBundle (testBundle);

        }

        beginTest ("Usage when containing other bundles (recursively)");
        {
            OSCBundle complexTestBundle;
            complexTestBundle.addElement (generateTestBundle());
            complexTestBundle.addElement (OSCMessage ("/test/"));
            complexTestBundle.addElement (generateTestBundle());

            expect (complexTestBundle.size() == 3);

            OSCBundle::Element* elements = complexTestBundle.begin();

            expect (! elements[0].isMessage());
            expect (elements[0].isBundle());
            expect (elements[1].isMessage());
            expect (! elements[1].isBundle());
            expect (! elements[2].isMessage());
            expect (elements[2].isBundle());

            expectBundleEqualsTestBundle (elements[0].getBundle());
            expect (elements[1].getMessage().size() == 0);
            expect (elements[1].getMessage().getAddressPattern().toString() == "/test");
            expectBundleEqualsTestBundle (elements[2].getBundle());
        }
    }

private:

    int testInt = 127;
    float testFloat = 1.5;

    OSCBundle generateTestBundle()
    {
        OSCBundle bundle;

        OSCMessage msg1 ("/test/fader");
        msg1.addInt32 (testInt);

        OSCMessage msg2 ("/test/foo");
        msg2.addString ("bar");
        msg2.addFloat32 (testFloat);

        bundle.addElement (msg1);
        bundle.addElement (msg2);

        return bundle;
    }

    void expectBundleEqualsTestBundle (const OSCBundle& bundle)
    {
        expect (bundle.size() == 2);
        expect (bundle[0].isMessage());
        expect (! bundle[0].isBundle());
        expect (bundle[1].isMessage());
        expect (! bundle[1].isBundle());

        int numElementsCounted = 0;
        for (auto& element : bundle)
        {
            expect (element.isMessage());
            expect (! element.isBundle());
            ++numElementsCounted;
        }
        expectEquals (numElementsCounted, 2);

        auto* e = bundle.begin();
        expect (e[0].getMessage().size() == 1);
        expect (e[0].getMessage().begin()->getInt32() == testInt);
        expect (e[1].getMessage().size() == 2);
        expectEquals (e[1].getMessage()[1].getFloat32(), testFloat);
    }
};

static OSCBundleTests OSCBundleUnitTests;

//==============================================================================
class OSCBundleElementTests final : public UnitTest
{
public:
    OSCBundleElementTests()
        : UnitTest ("OSCBundle::Element class", UnitTestCategories::osc)
    {}

    void runTest() override
    {
        beginTest ("Construction from OSCMessage");
        {
            float testFloat = -0.125;
            OSCMessage msg ("/test");
            msg.addFloat32 (testFloat);

            OSCBundle::Element element (msg);

            expect (element.isMessage());
            expect (element.getMessage().size() == 1);
            expect (element.getMessage()[0].getType() == OSCTypes::float32);
            expectEquals (element.getMessage()[0].getFloat32(), testFloat);
        }
    }
};

static OSCBundleElementTests OSCBundleElementUnitTests;

#endif

} // namespace juce
