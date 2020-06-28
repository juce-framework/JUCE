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

#pragma once


//==============================================================================
struct LicenseState
{
    enum class Type
    {
        none,
        gpl,
        personal,
        educational,
        indie,
        pro
    };

    LicenseState() = default;

    LicenseState (Type t, int v, String user, String token)
        : type (t), version (v), username (user), authToken (token)
    {
    }

    bool operator== (const LicenseState& other) const noexcept
    {
        return type == other.type
              && version == other.version
              && username == other.username
              && authToken == other.authToken;
    }

    bool operator != (const LicenseState& other) const noexcept
    {
        return ! operator== (other);
    }

    bool isSignedIn() const noexcept    { return isGPL() || (version > 0 && username.isNotEmpty()); }
    bool isOldLicense() const noexcept  { return isSignedIn() && version < projucerMajorVersion; }
    bool isGPL()       const noexcept   { return type == Type::gpl; }

    bool canUnlockFullFeatures() const noexcept
    {
        return isGPL() || (isSignedIn() && ! isOldLicense() && (type == Type::indie || type == Type::pro));
    }

    String getLicenseTypeString() const
    {
        switch (type)
        {
            case Type::none:         return "No license";
            case Type::gpl:          return "GPL";
            case Type::personal:     return "Personal";
            case Type::educational:  return "Educational";
            case Type::indie:        return "Indie";
            case Type::pro:          return "Pro";
            default:                 break;
        };

        jassertfalse;
        return {};
    }

    Type type = Type::none;
    int version = -1;
    String username, authToken;
};
