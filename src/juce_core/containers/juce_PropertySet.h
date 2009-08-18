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

#ifndef __JUCE_PROPERTYSET_JUCEHEADER__
#define __JUCE_PROPERTYSET_JUCEHEADER__

#include "../text/juce_StringPairArray.h"
#include "../text/juce_XmlElement.h"


//==============================================================================
/**
    A set of named property values, which can be strings, integers, floating point, etc.

    Effectively, this just wraps a StringPairArray in an interface that makes it easier
    to load and save types other than strings.

    See the PropertiesFile class for a subclass of this, which automatically broadcasts change
    messages and saves/loads the list from a file.
*/
class JUCE_API  PropertySet
{
public:
    //==============================================================================
    /** Creates an empty PropertySet.

        @param ignoreCaseOfKeyNames         if true, the names of properties are compared in a
                                            case-insensitive way
    */
    PropertySet (const bool ignoreCaseOfKeyNames = false) throw();

    /** Creates a copy of another PropertySet.
    */
    PropertySet (const PropertySet& other) throw();

    /** Copies another PropertySet over this one.
    */
    const PropertySet& operator= (const PropertySet& other) throw();

    /** Destructor. */
    virtual ~PropertySet();

    //==============================================================================
    /** Returns one of the properties as a string.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
        @param defaultReturnValue   a value to return if the named property doesn't actually exist
    */
    const String getValue (const String& keyName,
                           const String& defaultReturnValue = String::empty) const throw();

    /** Returns one of the properties as an integer.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
        @param defaultReturnValue   a value to return if the named property doesn't actually exist
    */
    int getIntValue (const String& keyName,
                     const int defaultReturnValue = 0) const throw();

    /** Returns one of the properties as an double.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
        @param defaultReturnValue   a value to return if the named property doesn't actually exist
    */
    double getDoubleValue (const String& keyName,
                           const double defaultReturnValue = 0.0) const throw();

    /** Returns one of the properties as an boolean.

        The result will be true if the string found for this key name can be parsed as a non-zero
        integer.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
        @param defaultReturnValue   a value to return if the named property doesn't actually exist
    */
    bool getBoolValue (const String& keyName,
                       const bool defaultReturnValue = false) const throw();

    /** Returns one of the properties as an XML element.

        The result will a new XMLElement object that the caller must delete. If may return 0 if the
        key isn't found, or if the entry contains an string that isn't valid XML.

        If the value isn't found in this set, then this will look for it in a fallback
        property set (if you've specified one with the setFallbackPropertySet() method),
        and if it can't find one there, it'll return the default value passed-in.

        @param keyName              the name of the property to retrieve
    */
    XmlElement* getXmlValue (const String& keyName) const;

    //==============================================================================
    /** Sets a named property as a string.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
    */
    void setValue (const String& keyName, const String& value) throw();

    /** Sets a named property as a string.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
    */
    void setValue (const String& keyName, const tchar* const value) throw();

    /** Sets a named property to an integer.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
    */
    void setValue (const String& keyName, const int value) throw();

    /** Sets a named property to a double.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
    */
    void setValue (const String& keyName, const double value) throw();

    /** Sets a named property to a boolean.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param value        the new value to set it to
    */
    void setValue (const String& keyName, const bool value) throw();

    /** Sets a named property to an XML element.

        @param keyName      the name of the property to set. (This mustn't be an empty string)
        @param xml          the new element to set it to. If this is zero, the value will be set to
                            an empty string
        @see getXmlValue
    */
    void setValue (const String& keyName, const XmlElement* const xml);

    //==============================================================================
    /** Deletes a property.

        @param keyName      the name of the property to delete. (This mustn't be an empty string)
    */
    void removeValue (const String& keyName) throw();

    /** Returns true if the properies include the given key. */
    bool containsKey (const String& keyName) const throw();

    /** Removes all values. */
    void clear();

    //==============================================================================
    /** Returns the keys/value pair array containing all the properties. */
    StringPairArray& getAllProperties() throw()                         { return properties; }

    /** Returns the lock used when reading or writing to this set */
    const CriticalSection& getLock() const throw()                      { return lock; }

    //==============================================================================
    /** Returns an XML element which encapsulates all the items in this property set.

        The string parameter is the tag name that should be used for the node.

        @see restoreFromXml
    */
    XmlElement* createXml (const String& nodeName) const throw();

    /** Reloads a set of properties that were previously stored as XML.

        The node passed in must have been created by the createXml() method.

        @see createXml
    */
    void restoreFromXml (const XmlElement& xml) throw();

    //==============================================================================
    /** Sets up a second PopertySet that will be used to look up any values that aren't
        set in this one.

        If you set this up to be a pointer to a second property set, then whenever one
        of the getValue() methods fails to find an entry in this set, it will look up that
        value in the fallback set, and if it finds it, it will return that.

        Make sure that you don't delete the fallback set while it's still being used by
        another set! To remove the fallback set, just call this method with a null pointer.

        @see getFallbackPropertySet
    */
    void setFallbackPropertySet (PropertySet* fallbackProperties) throw();

    /** Returns the fallback property set.
        @see setFallbackPropertySet
    */
    PropertySet* getFallbackPropertySet() const throw()                 { return fallbackProperties; }

    //==============================================================================
    juce_UseDebuggingNewOperator


protected:
    //==============================================================================
    /** Subclasses can override this to be told when one of the properies has been changed.
    */
    virtual void propertyChanged();


private:
    //==============================================================================
    StringPairArray properties;
    PropertySet* fallbackProperties;
    CriticalSection lock;
    bool ignoreCaseOfKeys;
};


#endif   // __JUCE_PROPERTYSET_JUCEHEADER__
