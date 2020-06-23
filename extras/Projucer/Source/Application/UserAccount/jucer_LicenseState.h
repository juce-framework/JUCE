/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

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
