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

#pragma once

#include <regex>

//==============================================================================
class XcodeProjectParser
{
public:
    //==============================================================================
    static std::unique_ptr<HashMap<std::string, std::string>> parseObjects (const File& projectFile)
    {
        auto pbxprojs = projectFile.findChildFiles (File::TypesOfFileToFind::findFiles, false, "*.pbxproj");

        if (pbxprojs.isEmpty())
        {
            jassertfalse;
            return nullptr;
        }

        auto content = pbxprojs[0].loadFileAsString().toStdString();

        std::regex comments ("/\\*.*?\\*/");
        std::string empty ("");
        content = (std::regex_replace (content, comments, empty));

        std::regex whitespace ("\\s+");
        std::string space (" ");
        content = (std::regex_replace (content, whitespace, space));

        auto objects = std::make_unique<HashMap<std::string, std::string>>();
        std::smatch objectsStartMatch;

        if (! std::regex_search (content, objectsStartMatch, std::regex ("[ ;{]+objects *= *\\{")))
        {
            jassertfalse;
            return nullptr;
        }

        auto strPtr = content.begin() + objectsStartMatch.position() + objectsStartMatch.length();

        while (strPtr++ != content.end())
        {
            if (*strPtr == ' ' || *strPtr == ';')
                continue;

            if (*strPtr == '}')
                break;

            auto groupReference = parseObjectID (content, strPtr);

            if (groupReference.empty())
            {
                jassertfalse;
                return nullptr;
            }

            while (*strPtr == ' ' || *strPtr == '=')
            {
                if (++strPtr == content.end())
                {
                    jassertfalse;
                    return nullptr;
                }
            }

            auto bracedContent = parseBracedContent (content, strPtr);

            if (bracedContent.empty())
                return nullptr;

            objects->set (groupReference, bracedContent);
        }

        jassert (strPtr <= content.end());

        return objects;
    }

    static std::pair<std::string, std::string> findObjectMatching (const HashMap<std::string, std::string>& objects,
                                                                   const std::regex& rgx)
    {
        HashMap<std::string, std::string>::Iterator it (objects);
        std::smatch match;

        while (it.next())
        {
            auto key = it.getValue();

            if (std::regex_search (key, match, rgx))
                return { it.getKey(), it.getValue() };
        }

        return {};
    }

    //==============================================================================
    struct BuildProduct
    {
        String name;
        String path;
    };

    static std::vector<BuildProduct> parseBuildProducts (const File& projectFile)
    {
        auto objects = parseObjects (projectFile);

        if (objects == nullptr)
            return {};

        auto mainObject = findObjectMatching (*objects, std::regex ("[ ;{]+isa *= *PBXProject[ ;}]+"));
        jassert (! mainObject.first.empty());

        auto targetRefs = parseObjectItemList (mainObject.second, "targets");
        jassert (! targetRefs.isEmpty());

        std::vector<BuildProduct> results;

        for (auto& t : targetRefs)
        {
            auto targetRef = t.toStdString();

            if (! objects->contains (targetRef))
            {
                jassertfalse;
                continue;
            }

            auto name = parseObjectItemValue (objects->getReference (targetRef), "name");

            if (name.empty())
                continue;

            auto productRef = parseObjectItemValue (objects->getReference (targetRef), "productReference");

            if (productRef.empty())
                continue;

            if (! objects->contains (productRef))
            {
                jassertfalse;
                continue;
            }

            auto path = parseObjectItemValue (objects->getReference (productRef), "path");

            if (path.empty())
                continue;

            results.push_back ({ String (name).unquoted(), String (path).unquoted() });
        }

        return results;
    }

private:
    //==============================================================================
    static std::string parseObjectID (std::string& content, std::string::iterator& ptr)
    {
        auto start = ptr;

        while (ptr != content.end() && *ptr != ' ' && *ptr != ';' && *ptr != '=')
            ++ptr;

        return ptr == content.end() ? std::string()
        : content.substr ((size_t) std::distance (content.begin(), start),
                          (size_t) std::distance (start, ptr));
    }

    //==============================================================================
    static std::string parseBracedContent (std::string& content, std::string::iterator& ptr)
    {
        jassert (*ptr == '{');
        auto start = ++ptr;
        auto braceDepth = 1;

        while (ptr++ != content.end())
        {
            switch (*ptr)
            {
                case '{':
                    ++braceDepth;
                    break;

                case '}':
                    if (--braceDepth == 0)
                        return content.substr ((size_t) std::distance (content.begin(), start),
                                               (size_t) std::distance (start, ptr));

                default:
                    break;
            }
        }

        jassertfalse;
        return {};
    }

    //==============================================================================
    static std::string parseObjectItemValue (const std::string& source, const std::string& key)
    {
        std::smatch match;

        if (! std::regex_search (source, match, std::regex ("[ ;{]+" + key + " *= *(.*?) *;")))
        {
            jassertfalse;
            return {};
        }

        return match[1];
    }

    //==============================================================================
    static StringArray parseObjectItemList (const std::string& source, const std::string& key)
    {
        std::smatch match;

        if (! std::regex_search (source, match, std::regex ("[ ;{]+" + key + " *= *\\((.*?)\\)")))
        {
            jassertfalse;
            return {};
        }

        auto result = StringArray::fromTokens (String (match[1]), ", ", "");
        result.removeEmptyStrings();

        return result;
    }
};
