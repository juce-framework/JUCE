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

/** An abstract interface representing the value of an accessibility element.

    Values should be used when information needs to be conveyed which cannot
    be represented by the accessibility element's label alone. For example, a
    gain slider with the label "Gain" needs to also provide a value for its
    position whereas a "Save" button does not.

    This class allows for full control over the value text/numeric conversion,
    ranged, and read-only properties but in most cases you'll want to use one
    of the derived classes below which handle some of this for you.

    @see AccessibilityTextValueInterface, AccessibilityNumericValueInterface,
         AccessibilityRangedNumericValueInterface

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityValueInterface
{
public:
    /** Destructor. */
    virtual ~AccessibilityValueInterface() = default;

    /** Returns true if the value is read-only and cannot be modified by an
        accessibility client.

        @see setValue, setValueAsString
    */
    virtual bool isReadOnly() const = 0;

    /** Returns the current value as a double. */
    virtual double getCurrentValue() const = 0;

    /** Returns the current value as a String. */
    virtual String getCurrentValueAsString() const = 0;

    /** Sets the current value to a new double value. */
    virtual void setValue (double newValue) = 0;

    /** Sets the current value to a new String value. */
    virtual void setValueAsString (const String& newValue) = 0;

    /** Represents the range of this value, if supported.

        Return one of these from the `getRange()` method, providing a minimum,
        maximum, and interval value for the range to indicate that this is a
        ranged value.

        The default state is an "invalid" range, indicating that the accessibility
        element does not support ranged values.

        @see AccessibilityRangedNumericValueInterface

        @tags{Accessibility}
    */
    class JUCE_API  AccessibleValueRange
    {
    public:
        /** Constructor.

            Creates a default, "invalid" range that can be returned from
            `AccessibilityValueInterface::getRange()` to indicate that the value
            interface does not support ranged values.
        */
        AccessibleValueRange() = default;

        /** The minimum and maximum values for this range, inclusive. */
        struct JUCE_API  MinAndMax  { double min, max; };

        /** Constructor.

            Creates a valid AccessibleValueRange with the provided minimum, maximum,
            and interval values.
        */
        AccessibleValueRange (MinAndMax valueRange, double interval)
            : valid (true),
              range (valueRange),
              stepSize (interval)
        {
            jassert (range.min < range.max);
        }

        /** Returns true if this represents a valid range. */
        bool isValid() const noexcept            { return valid; }

        /** Returns the minimum value for this range. */
        double getMinimumValue() const noexcept  { return range.min; }

        /** Returns the maximum value for this range. */
        double getMaximumValue() const noexcept  { return range.max; }

        /** Returns the interval for this range. */
        double getInterval() const noexcept      { return stepSize; }

    private:
        bool valid = false;
        MinAndMax range {};
        double stepSize = 0.0;
    };

    /** If this is a ranged value, this should return a valid AccessibleValueRange
        object representing the supported numerical range.
    */
    virtual AccessibleValueRange getRange() const = 0;
};

//==============================================================================
/** A value interface that represents a text value.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityTextValueInterface  : public AccessibilityValueInterface
{
public:
    /** Returns true if the value is read-only and cannot be modified by an
        accessibility client.

        @see setValueAsString
    */
    bool isReadOnly() const override = 0;

    /** Returns the current value. */
    String getCurrentValueAsString() const override = 0;

    /** Sets the current value to a new value. */
    void setValueAsString (const String& newValue) override = 0;

    /** @internal */
    double getCurrentValue() const final         { return getCurrentValueAsString().getDoubleValue(); }
    /** @internal */
    void setValue (double newValue) final        { setValueAsString (String (newValue)); }
    /** @internal */
    AccessibleValueRange getRange() const final  { return {}; }
};

//==============================================================================
/** A value interface that represents a non-ranged numeric value.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityNumericValueInterface  : public AccessibilityValueInterface
{
public:
    /** Returns true if the value is read-only and cannot be modified by an
        accessibility client.

        @see setValue
    */
    bool isReadOnly() const override = 0;

    /** Returns the current value. */
    double getCurrentValue() const override = 0;

    /** Sets the current value to a new value. */
    void setValue (double newValue) override = 0;

    /** @internal */
    String getCurrentValueAsString() const final          { return String (getCurrentValue()); }
    /** @internal */
    void setValueAsString (const String& newValue) final  { setValue (newValue.getDoubleValue()); }
    /** @internal */
    AccessibleValueRange getRange() const final           { return {}; }
};

//==============================================================================
/** A value interface that represents a ranged numeric value.

    @tags{Accessibility}
*/
class JUCE_API  AccessibilityRangedNumericValueInterface  : public AccessibilityValueInterface
{
public:
    /** Returns true if the value is read-only and cannot be modified by an
        accessibility client.

        @see setValueAsString
    */
    bool isReadOnly() const override = 0;

    /** Returns the current value. */
    double getCurrentValue() const override = 0;

    /** Sets the current value to a new value. */
    void setValue (double newValue) override = 0;

    /** Returns the range. */
    AccessibleValueRange getRange() const override = 0;

    /** @internal */
    String getCurrentValueAsString() const final          { return String (getCurrentValue()); }
    /** @internal */
    void setValueAsString (const String& newValue) final  { setValue (newValue.getDoubleValue()); }
};

} // namespace juce
