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

//==============================================================================
/**
    An OSC bundle.

    An OSCBundle contains an OSCTimeTag and zero or more OSCBundle Elements.
    The elements of a bundle can be OSC messages or other OSC bundles (this
    means that OSC bundles can be nested).

    This is an advanced OSC structure useful to bundle OSC messages together
    whose effects must occur simultaneously at some given time. For most
    use cases it is probably enough to send and receive plain OSC messages.

    @tags{OSC}
*/
class JUCE_API  OSCBundle
{
public:
    //==============================================================================
    /** Constructs an OSCBundle with no content and a default time tag ("immediately"). */
    OSCBundle();

    /** Constructs an OSCBundle with no content and a given time tag. */
    OSCBundle (OSCTimeTag timeTag);

    //==============================================================================
    /** Sets the OSCBundle's OSC time tag. */
    void setTimeTag (OSCTimeTag newTimeTag) noexcept        { timeTag = newTimeTag; }

    /** Returns the OSCBundle's OSC time tag. */
    OSCTimeTag getTimeTag() const noexcept                  { return timeTag; }

    //==============================================================================
    /**
        An OSC bundle element.

        An OSCBundle Element contains either one OSCMessage or one OSCBundle.
     */
    class JUCE_API  Element
    {
    public:
        //==============================================================================
        /** Constructs an OSCBundle Element from an OSCMessage. */
        Element (OSCMessage message);

        /** Constructs an OSCBundle Element from an OSCBundle. */
        Element (OSCBundle bundle);

        /** Copy constructor. */
        Element (const Element& other);

        /** Destructor. */
        ~Element();

        /** Returns true if the OSCBundle element is an OSCMessage. */
        bool isMessage() const noexcept;

        /** Returns true if the OSCBundle element is an OSCBundle. */
        bool isBundle() const noexcept;

        /** Returns a reference to the contained OSCMessage.
            If the OSCBundle element is not an OSCMessage, behaviour is undefined.
         */
        const OSCMessage& getMessage() const;

        /** Returns a reference to the contained OSCBundle.
            If the OSCBundle element is not an OSCBundle, behaviour is undefined.
        */
        const OSCBundle& getBundle() const;

    private:
        //==============================================================================
        std::unique_ptr<OSCMessage> message;
        std::unique_ptr<OSCBundle> bundle;
    };

    //==============================================================================
    /** Returns the number of elements contained in the bundle. */
    int size() const noexcept                                    { return elements.size(); }

    /** Returns true if the bundle contains no elements; false otherwise. */
    bool isEmpty() const noexcept                                { return elements.isEmpty(); }

    /** Returns a reference to the OSCBundle element at index i in this bundle.
        This method does not check the range and results in undefined behaviour
        in case i < 0 or i >= size().
    */
    OSCBundle::Element& operator[] (const int i) noexcept
    {
        return elements.getReference (i);
    }

    const OSCBundle::Element& operator[] (const int i) const noexcept
    {
        return elements.getReference (i);
    }

    /** Adds an OSCBundleElement to the OSCBundle's content. s*/
    void addElement (const OSCBundle::Element& element)          { elements.add (element); }

    /** Returns a pointer to the first element of the OSCBundle. */
    OSCBundle::Element* begin() noexcept                         { return elements.begin(); }

    /** Returns a pointer to the first element of the OSCBundle. */
    const OSCBundle::Element* begin() const noexcept             { return elements.begin(); }

    /** Returns a pointer past the last element of the OSCBundle. */
    OSCBundle::Element* end() noexcept                           { return elements.end(); }

    /** Returns a pointer past the last element of the OSCBundle. */
    const OSCBundle::Element* end() const noexcept               { return elements.end(); }

private:
    //==============================================================================
    Array<OSCBundle::Element> elements;
    OSCTimeTag timeTag;
};

} // namespace juce
