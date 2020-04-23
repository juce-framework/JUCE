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

    LicenseState (Type t, String token, String user, Image avatarImage)
        : type (t), authToken (token), username (user), avatar (avatarImage)
    {
    }

    bool isValid() const noexcept      { return isGPL() || (type != Type::none && authToken.isNotEmpty() && username.isNotEmpty()); }

    bool isPaid()      const noexcept  { return type == Type::indie || type == Type::pro; }
    bool isGPL()       const noexcept  { return type == Type::gpl; }
    bool isPaidOrGPL() const noexcept  { return isPaid() || isGPL(); }

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
    String authToken, username;
    Image avatar;
};
