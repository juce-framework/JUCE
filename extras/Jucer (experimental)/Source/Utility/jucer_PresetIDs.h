/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_PRESETIDS_H_40E9E82E__
#define __JUCER_PRESETIDS_H_40E9E82E__


// Handy list of static Identifiers..
namespace Ids
{
    #define DECLARE_ID(name)      const Identifier name (#name)

    DECLARE_ID (text);
    DECLARE_ID (name);
    DECLARE_ID (file);
    DECLARE_ID (font);
    DECLARE_ID (mode);
    DECLARE_ID (type);
    DECLARE_ID (version);
    DECLARE_ID (position);
    DECLARE_ID (source);
    DECLARE_ID (readOnly);
    DECLARE_ID (editMode);
    DECLARE_ID (justification);
    DECLARE_ID (items);
    DECLARE_ID (editable);
    DECLARE_ID (textJustification);
    DECLARE_ID (unselectedText);
    DECLARE_ID (noItemsText);
    DECLARE_ID (min);
    DECLARE_ID (max);
    DECLARE_ID (width);
    DECLARE_ID (height);
    DECLARE_ID (background);
    DECLARE_ID (interval);
    DECLARE_ID (textBoxPos);
    DECLARE_ID (textBoxWidth);
    DECLARE_ID (textBoxHeight);
    DECLARE_ID (skew);
    DECLARE_ID (scrollBarV);
    DECLARE_ID (scrollBarH);
    DECLARE_ID (scrollbarWidth);
    DECLARE_ID (initialState);
    DECLARE_ID (scrollbarsShown);
    DECLARE_ID (caretVisible);
    DECLARE_ID (popupMenuEnabled);
    DECLARE_ID (radioGroup);
    DECLARE_ID (connectedLeft);
    DECLARE_ID (connectedRight);
    DECLARE_ID (connectedTop);
    DECLARE_ID (connectedBottom);
    DECLARE_ID (juceFolder);
    DECLARE_ID (targetFolder);
    DECLARE_ID (vstFolder);
    DECLARE_ID (rtasFolder);
    DECLARE_ID (auFolder);
    DECLARE_ID (extraCompilerFlags);
    DECLARE_ID (extraLinkerFlags);
    DECLARE_ID (extraDefs);
    DECLARE_ID (libraryName_Debug);
    DECLARE_ID (libraryName_Release);
    DECLARE_ID (libraryType);
    DECLARE_ID (isDebug);
    DECLARE_ID (targetName);
    DECLARE_ID (binaryPath);
    DECLARE_ID (optimisation);
    DECLARE_ID (defines);
    DECLARE_ID (headerPath);
    DECLARE_ID (osxSDK);
    DECLARE_ID (osxCompatibility);
    DECLARE_ID (jucerVersion);
    DECLARE_ID (projectType);
    DECLARE_ID (juceLinkage);
    DECLARE_ID (buildVST);
    DECLARE_ID (bundleIdentifier);
    DECLARE_ID (compile);
    DECLARE_ID (resource);
    DECLARE_ID (className);
    DECLARE_ID (classDesc);
    DECLARE_ID (controlPoint);
    DECLARE_ID (createCallback);
    DECLARE_ID (parentClasses);
    DECLARE_ID (constructorParams);
    DECLARE_ID (memberInitialisers);
    DECLARE_ID (lossOfFocusDiscardsChanges);
    DECLARE_ID (url);
    DECLARE_ID (rootItemVisible);
    DECLARE_ID (openByDefault);
    DECLARE_ID (locked);
    const Identifier class_ ("class");
    const Identifier id_ ("id");

    #undef DECLARE_ID
}

#endif  // __JUCER_PRESETIDS_H_40E9E82E__
