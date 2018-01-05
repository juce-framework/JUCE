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

namespace juce
{

//==============================================================================
/**
    This class acts as a wrapper around a property inside a ValueTree.

    If the property inside the ValueTree is missing or empty the ValueWithDefault will automatically
    return a default value, which can be specified when initialising the ValueWithDefault.
*/
class ValueWithDefault
{
public:
    //==============================================================================
    /** Creates an unitialised ValueWithDefault. Initialise it using one of the referTo() methods. */
    ValueWithDefault()    : undoManager (nullptr) {}

    /** Creates an ValueWithDefault object. The default value will be an empty var. */
    ValueWithDefault (ValueTree& tree, const Identifier& propertyID,
                      UndoManager* um)
        : targetTree (tree),
          targetProperty (propertyID),
          undoManager (um),
          defaultValue()
    {
    }

    /** Creates an ValueWithDefault object. The default value will be defaultToUse. */
    ValueWithDefault (ValueTree& tree, const Identifier& propertyID,
                      UndoManager* um, const var& defaultToUse)
        : targetTree (tree),
          targetProperty (propertyID),
          undoManager (um),
          defaultValue (defaultToUse)
    {
    }

    /** Creates a ValueWithDefault object from another ValueWithDefault object. */
    ValueWithDefault (const ValueWithDefault& other)
        : targetTree (other.targetTree),
          targetProperty (other.targetProperty),
          undoManager (other.undoManager),
          defaultValue (other.defaultValue)
    {
    }

    //==============================================================================
    /** Returns the current value of the property. If the property does not exist or is empty,
        returns the default value.
    */
    var get() const noexcept
    {
        if (isUsingDefault())
            return defaultValue;

        return targetTree[targetProperty];
    }

    /** Returns the current property as a Value object. */
    Value getPropertyAsValue()     { return targetTree.getPropertyAsValue (targetProperty, undoManager); }

    /** Returns the current default value. */
    var getDefault() const         { return defaultValue; }

    /** Sets the default value to a new var. */
    void setDefault (const var& newDefault)
    {
        if (defaultValue != newDefault)
            defaultValue = newDefault;
    }

    /** Returns true if the property does not exist or is empty. */
    bool isUsingDefault() const
    {
        return ! targetTree.hasProperty (targetProperty);
    }

    /** Resets the property to an empty var. */
    void resetToDefault() noexcept
    {
        targetTree.removeProperty (targetProperty, nullptr);
    }

    //==============================================================================
    /** Sets the property and returns the new ValueWithDefault. This will modify the property in the referenced ValueTree. */
    ValueWithDefault& operator= (const var& newValue)
    {
        setValue (newValue, undoManager);
        return *this;
    }

    /** Sets the property. This will actually modify the property in the referenced ValueTree. */
    void setValue (const var& newValue, UndoManager* undoManagerToUse)
    {
        targetTree.setProperty (targetProperty, newValue, undoManagerToUse);
    }

    //==============================================================================
    /** Makes the ValueWithDefault refer to the specified property inside the given ValueTree. */
    void referTo (ValueTree& tree, const Identifier& property, UndoManager* um)
    {
        referToWithDefault (tree, property, um, var());
    }

    /** Makes the ValueWithDefault refer to the specified property inside the given ValueTree,
        and specifies a default value to use.
     */
    void referTo (ValueTree& tree, const Identifier& property, UndoManager* um, const var& defaultVal)
    {
        referToWithDefault (tree, property, um, defaultVal);
    }

    //==============================================================================
    /** Returns a reference to the ValueTree containing the referenced property. */
    ValueTree& getValueTree() noexcept                      { return targetTree; }

    /** Returns the property ID of the referenced property. */
    Identifier& getPropertyID() noexcept                    { return targetProperty; }

private:
    //==============================================================================
    ValueTree targetTree;
    Identifier targetProperty;
    UndoManager* undoManager;
    var defaultValue;

    //==============================================================================
    void referToWithDefault (ValueTree& v, const Identifier& i, UndoManager* um, const var& defaultVal)
    {
        targetTree = v;
        targetProperty = i;
        undoManager = um;
        defaultValue = defaultVal;
    }
};

} // namespace juce
