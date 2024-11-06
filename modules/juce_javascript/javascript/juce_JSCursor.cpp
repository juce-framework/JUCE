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

JSCursor::JSCursor (JSObject rootIn) : root (std::move (rootIn))
{
}

var JSCursor::get() const
{
    if (const auto resolved = getFullResolution())
        return resolved->get();

    return var::undefined();
}

void JSCursor::set (const var& value) const
{
    const auto resolved = getPartialResolution();

    if (! resolved.has_value())
    {
        jassertfalse;  // Can't resolve an Object to change along the path stored in the cursor
        return;
    }

    const auto& [object, property] = *resolved;

    if (! property.has_value())
    {
        jassertfalse;  // Can't set the value of the root Object
        return;
    }

    if (auto* prop = std::get_if<Identifier> (&(*property)))
    {
        object.setProperty (*prop, value);
        return;
    }

    if (auto* prop = std::get_if<int64> (&(*property)))
    {
        object.setProperty (*prop, value);
        return;
    }
}

JSCursor JSCursor::getChild (const Identifier& name) const
{
    auto copy = *this;
    copy.path.emplace_back (name);
    return copy;
}

JSCursor JSCursor::operator[] (const Identifier& name) const
{
    return getChild (name);
}

JSCursor JSCursor::getChild (int64 index) const
{
    auto copy = *this;
    copy.path.emplace_back (index);
    return copy;
}

JSCursor JSCursor::operator[] (int64 index) const
{
    return getChild (index);
}

JSObject JSCursor::getOrCreateObject() const
{
    const auto resolved = getPartialResolution();
    jassert (resolved.has_value());

    const auto& [object, property] = *resolved;

    if (! property.has_value())
        return object;

    auto* integerValue = std::get_if<int64> (&(*property));

    jassert   (integerValue == nullptr
            || (object.isArray() && (*integerValue) < object.getSize()));

    if (integerValue != nullptr)
        return object[*integerValue];

    auto* prop = std::get_if<Identifier> (&(*property));
    jassert(prop != nullptr);
    return object[*prop];
}

bool JSCursor::isValid() const
{
    return getPartialResolution().has_value();
}

bool JSCursor::isArray() const
{
    if (auto resolved = getFullResolution())
        return resolved->isArray();

    return false;
}

var JSCursor::invoke (Span<const var> args, Result* result) const
{
    const auto resolved = getPartialResolution();

    if (! resolved.has_value())
    {
        jassertfalse;
        return {};
    }

    const auto& [object, property] = *resolved;
    if (! property.has_value())
    {
        jassertfalse;
        return {};
    }

    return object.invokeMethod (*std::get_if<Identifier> (&(*property)), args, result);
}

std::optional<JSObject> JSCursor::resolve (JSObject object, Property property)
{
    if (auto* index = std::get_if<int64> (&property))
    {
        if (! object.isArray())
            return std::nullopt;

        if (! (*index < object.getSize()))
            return std::nullopt;

        return object[*index];
    }

    if (auto* key = std::get_if<Identifier> (&property))
    {
        if (! object.hasProperty (*key))
            return std::nullopt;

        return object[*key];
    }

    jassertfalse;
    return std::nullopt;
}

std::optional<JSCursor::PartialResolution> JSCursor::getPartialResolution() const
{
    auto object = root;

    for (int i = 0, iEnd = (int) path.size() - 1; i < iEnd; ++i)
    {
        const auto& property = path[(size_t) i];
        auto objectOpt = resolve (object, property);

        if (! objectOpt.has_value())
            return std::nullopt;

        object = *objectOpt;
    }

    return std::make_optional<PartialResolution> (std::move (object),
                                                  path.empty() ? std::nullopt
                                                               : std::make_optional (path.back()));
}

std::optional<JSObject> JSCursor::getFullResolution() const
{
    if (auto partiallyResolved = getPartialResolution())
    {
        if (! partiallyResolved->second.has_value())
            return partiallyResolved->first;

        return resolve (partiallyResolved->first, *(partiallyResolved->second));
    }

    return std::nullopt;
}

} // namespace juce
