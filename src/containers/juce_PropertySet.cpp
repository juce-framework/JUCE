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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_PropertySet.h"
#include "../threads/juce_ScopedLock.h"
#include "../text/juce_XmlDocument.h"


//==============================================================================
PropertySet::PropertySet (const bool ignoreCaseOfKeyNames) throw()
    : properties (ignoreCaseOfKeyNames),
      fallbackProperties (0),
      ignoreCaseOfKeys (ignoreCaseOfKeyNames)
{
}

PropertySet::PropertySet (const PropertySet& other) throw()
    : properties (other.properties),
      fallbackProperties (other.fallbackProperties),
      ignoreCaseOfKeys (other.ignoreCaseOfKeys)
{
}

const PropertySet& PropertySet::operator= (const PropertySet& other) throw()
{
    properties = other.properties;
    fallbackProperties = other.fallbackProperties;
    ignoreCaseOfKeys = other.ignoreCaseOfKeys;

    propertyChanged();
    return *this;
}

PropertySet::~PropertySet()
{
}

void PropertySet::clear()
{
    const ScopedLock sl (lock);

    if (properties.size() > 0)
    {
        properties.clear();
        propertyChanged();
    }
}

const String PropertySet::getValue (const String& keyName,
                                    const String& defaultValue) const throw()
{
    const ScopedLock sl (lock);

    const int index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

    if (index >= 0)
        return properties.getAllValues() [index];

    return fallbackProperties != 0 ? fallbackProperties->getValue (keyName, defaultValue)
                                   : defaultValue;
}

int PropertySet::getIntValue (const String& keyName,
                              const int defaultValue) const throw()
{
    const ScopedLock sl (lock);
    const int index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

    if (index >= 0)
        return properties.getAllValues() [index].getIntValue();

    return fallbackProperties != 0 ? fallbackProperties->getIntValue (keyName, defaultValue)
                                   : defaultValue;
}

double PropertySet::getDoubleValue (const String& keyName,
                                    const double defaultValue) const throw()
{
    const ScopedLock sl (lock);
    const int index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

    if (index >= 0)
        return properties.getAllValues()[index].getDoubleValue();

    return fallbackProperties != 0 ? fallbackProperties->getDoubleValue (keyName, defaultValue)
                                   : defaultValue;
}

bool PropertySet::getBoolValue (const String& keyName,
                                const bool defaultValue) const throw()
{
    const ScopedLock sl (lock);
    const int index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

    if (index >= 0)
        return properties.getAllValues() [index].getIntValue() != 0;

    return fallbackProperties != 0 ? fallbackProperties->getBoolValue (keyName, defaultValue)
                                   : defaultValue;
}

XmlElement* PropertySet::getXmlValue (const String& keyName) const
{
    XmlDocument doc (getValue (keyName));

    return doc.getDocumentElement();
}

void PropertySet::setValue (const String& keyName,
                            const String& value) throw()
{
    jassert (keyName.isNotEmpty()); // shouldn't use an empty key name!

    if (keyName.isNotEmpty())
    {
        const ScopedLock sl (lock);

        const int index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

        if (index < 0 || properties.getAllValues() [index] != value)
        {
            properties.set (keyName, value);
            propertyChanged();
        }
    }
}

void PropertySet::removeValue (const String& keyName) throw()
{
    if (keyName.isNotEmpty())
    {
        const ScopedLock sl (lock);
        const int index = properties.getAllKeys().indexOf (keyName, ignoreCaseOfKeys);

        if (index >= 0)
        {
            properties.remove (keyName);
            propertyChanged();
        }
    }
}

void PropertySet::setValue (const String& keyName, const tchar* const value) throw()
{
    setValue (keyName, String (value));
}

void PropertySet::setValue (const String& keyName, const int value) throw()
{
    setValue (keyName, String (value));
}

void PropertySet::setValue (const String& keyName, const double value) throw()
{
    setValue (keyName, String (value));
}

void PropertySet::setValue (const String& keyName, const bool value) throw()
{
    setValue (keyName, String ((value) ? T("1") : T("0")));
}

void PropertySet::setValue (const String& keyName, const XmlElement* const xml)
{
    setValue (keyName, (xml == 0) ? String::empty
                                  : xml->createDocument (String::empty, true));
}

bool PropertySet::containsKey (const String& keyName) const throw()
{
    const ScopedLock sl (lock);
    return properties.getAllKeys().contains (keyName, ignoreCaseOfKeys);
}

void PropertySet::setFallbackPropertySet (PropertySet* fallbackProperties_) throw()
{
    const ScopedLock sl (lock);
    fallbackProperties = fallbackProperties_;
}

XmlElement* PropertySet::createXml (const String& nodeName) const throw()
{
    const ScopedLock sl (lock);
    XmlElement* const xml = new XmlElement (nodeName);

    for (int i = 0; i < properties.getAllKeys().size(); ++i)
    {
        XmlElement* const e = new XmlElement (T("VALUE"));

        e->setAttribute (T("name"), properties.getAllKeys()[i]);
        e->setAttribute (T("val"), properties.getAllValues()[i]);

        xml->addChildElement (e);
    }

    return xml;
}

void PropertySet::restoreFromXml (const XmlElement& xml) throw()
{
    const ScopedLock sl (lock);
    clear();

    forEachXmlChildElementWithTagName (xml, e, T("VALUE"))
    {
        if (e->hasAttribute (T("name"))
             && e->hasAttribute (T("val")))
        {
            properties.set (e->getStringAttribute (T("name")),
                            e->getStringAttribute (T("val")));
        }
    }

    if (properties.size() > 0)
        propertyChanged();
}

void PropertySet::propertyChanged()
{
}

END_JUCE_NAMESPACE
