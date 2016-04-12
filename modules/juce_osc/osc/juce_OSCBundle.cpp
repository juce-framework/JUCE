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
            message = new OSCMessage (other.getMessage());
        else
            bundle = new OSCBundle (other.getBundle());
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
#if JUCE_UNIT_TESTS

class OSCBundleTests  : public UnitTest
{
public:
    OSCBundleTests() : UnitTest ("OSCBundle class") {}

    void runTest()
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
        for (OSCBundle::Element* element = bundle.begin(); element != bundle.end(); ++element)
        {
            expect (element->isMessage());
            expect (! element->isBundle());
            ++numElementsCounted;
        }
        expectEquals (numElementsCounted, 2);

        OSCBundle::Element* e = bundle.begin();
        expect (e[0].getMessage().size() == 1);
        expect (e[0].getMessage().begin()->getInt32() == testInt);
        expect (e[1].getMessage().size() == 2);
        expect (e[1].getMessage()[1].getFloat32() == testFloat);
    }
};

static OSCBundleTests OSCBundleUnitTests;

//==============================================================================
class OSCBundleElementTests  : public UnitTest
{
public:
    OSCBundleElementTests() : UnitTest ("OSCBundle::Element class") {}

    void runTest()
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
            expect (element.getMessage()[0].getFloat32() == testFloat);
        }
    }
};

static OSCBundleElementTests OSCBundleElementUnitTests;

#endif // JUCE_UNIT_TESTS
