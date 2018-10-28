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
        content = (std::regex_replace (content, comments, ""));

        std::regex whitespace ("\\s+");
        content = (std::regex_replace (content, whitespace, " "));

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
    static std::vector<std::pair<String, String>> parseBuildProducts (const File& projectFile)
    {
        auto objects = parseObjects (projectFile);

        if (objects == nullptr)
            return {};

        auto mainObject = findObjectMatching (*objects, std::regex ("[ ;{]+isa *= *PBXProject[ ;}]+"));
        jassert (! mainObject.first.empty());

        auto targetRefs = parseObjectItemList (mainObject.second, "targets");
        jassert (! targetRefs.isEmpty());

        std::vector<std::pair<String, String>> results;

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
            }
        }

        jassertfalse;
        return {};
    }

    //==============================================================================
    static std::string parseObjectItemValue (const std::string& source, const std::string& key)
    {
        std::smatch match;

        if (! std::regex_search (source, match, std::regex ("[ ;{]+" + key + " *= *(.*?)[ ;]+")))
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
