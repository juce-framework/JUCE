/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    This class acts as a wrapper around a property inside a ValueTree.

    If the property inside the ValueTree is missing it will return a default value,
    which can be specified in the constructor or by calling setDefault().

    @tags{DataStructures}
*/
class JUCE_API  ValueTreePropertyWithDefault  : private Value::Listener
{
public:
    //==============================================================================
    /** Creates an uninitialised ValueTreePropertyWithDefault object.

        Initialise it using one of the referTo() methods.
    */
    ValueTreePropertyWithDefault() = default;

    /** Creates a ValueTreePropertyWithDefault object for the specified property.

        The default value will be an empty var.
    */
    ValueTreePropertyWithDefault (ValueTree& tree,
                                  const Identifier& propertyID,
                                  UndoManager* um)
    {
        referTo (tree, propertyID, um);
    }

    /** Creates an ValueTreePropertyWithDefault object for the specified property.

        The default value will be defaultToUse.
    */
    ValueTreePropertyWithDefault (ValueTree& tree,
                                  const Identifier& propertyID,
                                  UndoManager* um,
                                  var defaultToUse)
    {
        referTo (tree, propertyID, um, defaultToUse);
    }

    /** Creates a ValueTreePropertyWithDefault object for the specified property.

        The default value will be defaultToUse.

        Use this constructor if the underlying var object being controlled is an array and
        it will handle the conversion to/from a delimited String that can be written to
        XML format.
    */
    ValueTreePropertyWithDefault (ValueTree& tree,
                                  const Identifier& propertyID,
                                  UndoManager* um,
                                  var defaultToUse,
                                  StringRef arrayDelimiter)
    {
        referTo (tree, propertyID, um, defaultToUse, arrayDelimiter);
    }

    /** Creates a ValueTreePropertyWithDefault object from another ValueTreePropertyWithDefault object. */
    ValueTreePropertyWithDefault (const ValueTreePropertyWithDefault& other)
    {
        referToWithDefault (other.targetTree,
                            other.targetProperty,
                            other.undoManager,
                            other.defaultValue,
                            other.delimiter);
    }

    /** Destructor. */
    ~ValueTreePropertyWithDefault() override
    {
        defaultValue.removeListener (this);
    }

    //==============================================================================
    /** Returns the current value of the property.

        If the property does not exist this returns the default value.
    */
    var get() const noexcept
    {
        if (isUsingDefault())
            return defaultValue;

        if (delimiter.isNotEmpty())
            return delimitedStringToVarArray (targetTree[targetProperty].toString(), delimiter);

        return targetTree[targetProperty];
    }

    /** Returns the current property as a Value object. */
    Value getPropertyAsValue()               { return targetTree.getPropertyAsValue (targetProperty, undoManager); }

    /** Returns the current default value. */
    var getDefault() const                   { return defaultValue; }

    /** Sets the default value to a new var. */
    void setDefault (const var& newDefault)  { defaultValue = newDefault; }

    /** Returns true if the property does not exist in the referenced ValueTree. */
    bool isUsingDefault() const              { return ! targetTree.hasProperty (targetProperty); }

    /** Removes the property from the referenced ValueTree. */
    void resetToDefault() noexcept           { targetTree.removeProperty (targetProperty, nullptr); }

    /** You can assign a lambda to this callback and it will called when the default
        value is changed.

        @see setDefault
    */
    std::function<void()> onDefaultChange;

    //==============================================================================
    /** Sets the property and returns the new ValueTreePropertyWithDefault.

        This will modify the property in the referenced ValueTree.
    */
    ValueTreePropertyWithDefault& operator= (const var& newValue)
    {
        setValue (newValue, undoManager);
        return *this;
    }

    /** Sets the property.

        This will modify the property in the referenced ValueTree.
    */
    void setValue (const var& newValue, UndoManager* undoManagerToUse)
    {
        if (auto* array = newValue.getArray())
            targetTree.setProperty (targetProperty, varArrayToDelimitedString (*array, delimiter), undoManagerToUse);
        else
            targetTree.setProperty (targetProperty, newValue, undoManagerToUse);
    }

    //==============================================================================
    /** Makes the ValueTreePropertyWithDefault refer to the specified property inside
        the given ValueTree.

        The default value will be an empty var.
    */
    void referTo (ValueTree tree,
                  const Identifier& property,
                  UndoManager* um)
    {
        referToWithDefault (tree,
                            property,
                            um,
                            Value (new SynchronousValueSource (var())),
                            {});
    }

    /** Makes the ValueTreePropertyWithDefault refer to the specified property inside
        the given ValueTree.

        The default value will be defaultVal.
    */
    void referTo (ValueTree tree,
                  const Identifier& property,
                  UndoManager* um,
                  var defaultVal)
    {
        referToWithDefault (tree,
                            property,
                            um,
                            Value (new SynchronousValueSource (defaultVal)),
                            {});
    }

    /** Makes the ValueTreePropertyWithDefault refer to the specified property inside
        the given ValueTree.

        The default value will be defaultVal.
    */
    void referTo (ValueTree tree,
                  const Identifier& property,
                  UndoManager* um,
                  var defaultVal,
                  StringRef arrayDelimiter)
    {
        referToWithDefault (tree,
                            property,
                            um,
                            Value (new SynchronousValueSource (defaultVal)),
                            arrayDelimiter);
    }

    //==============================================================================
    /** Returns a reference to the ValueTree containing the referenced property. */
    ValueTree& getValueTree() noexcept                      { return targetTree; }

    /** Returns the property ID of the referenced property. */
    Identifier& getPropertyID() noexcept                    { return targetProperty; }

    /** Returns the UndoManager that is being used. */
    UndoManager* getUndoManager() noexcept                  { return undoManager; }

    //==============================================================================
    ValueTreePropertyWithDefault& operator= (const ValueTreePropertyWithDefault& other)
    {
        referToWithDefault (other.targetTree,
                            other.targetProperty,
                            other.undoManager,
                            other.defaultValue,
                            other.delimiter);

        return *this;
    }

private:
    //==============================================================================
    class SynchronousValueSource  : public Value::ValueSource
    {
    public:
        explicit SynchronousValueSource (const var& initialValue)
            : value (initialValue)
        {
        }

        var getValue() const override
        {
            return value;
        }

        void setValue (const var& newValue) override
        {
            if (! newValue.equalsWithSameType (value))
            {
                value = newValue;
                sendChangeMessage (true);
            }
        }

    private:
        var value;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynchronousValueSource)
    };

    //==============================================================================
    static String varArrayToDelimitedString (const Array<var>& input, StringRef delim)
    {
        // if you are trying to control a var that is an array then you need to
        // set a delimiter string that will be used when writing to XML!
        jassert (delim.isNotEmpty());

        StringArray elements;

        for (auto& v : input)
            elements.add (v.toString());

        return elements.joinIntoString (delim);
    }

    static Array<var> delimitedStringToVarArray (StringRef input, StringRef delim)
    {
        Array<var> arr;

        for (auto t : StringArray::fromTokens (input, delim, {}))
            arr.add (t);

        return arr;
    }

    void valueChanged (Value&) override
    {
        if (onDefaultChange != nullptr)
            onDefaultChange();
    }

    void referToWithDefault (ValueTree v,
                             const Identifier& i,
                             UndoManager* um,
                             const Value& defaultVal,
                             StringRef del)
    {
        targetTree = v;
        targetProperty = i;
        undoManager = um;
        defaultValue.referTo (defaultVal);
        delimiter = del;

        defaultValue.addListener (this);
    }

    //==============================================================================
    ValueTree targetTree;
    Identifier targetProperty;
    UndoManager* undoManager = nullptr;
    Value defaultValue;
    String delimiter;

    //==============================================================================
    JUCE_LEAK_DETECTOR (ValueTreePropertyWithDefault)
};

//==============================================================================
#ifndef DOXYGEN
using ValueWithDefault  [[deprecated ("This class has been renamed to better describe what is does. "
                                      "This declaration is here for backwards compatibility and new "
                                      "code should use the new class name.")]]
    = ValueTreePropertyWithDefault;
#endif

} // namespace juce
