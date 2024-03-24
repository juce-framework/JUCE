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

namespace juce::build_tools
{

    //==============================================================================
    /** Manipulates a cross-platform partial file path. (Needed because File is designed
        for absolute paths on the active OS)
    */
    class RelativePath
    {
    public:
        //==============================================================================
        enum RootFolder
        {
            unknown,
            projectFolder,
            buildTargetFolder
        };

        //==============================================================================
        RelativePath()
            : root (unknown)
        {}

        RelativePath (const String& relPath, const RootFolder rootType)
            : path (unixStylePath (relPath)), root (rootType)
        {}

        RelativePath (const File& file, const File& rootFolder, const RootFolder rootType)
            : path (unixStylePath (getRelativePathFrom (file, rootFolder))), root (rootType)
        {}

        RootFolder getRoot() const                              { return root; }

        String toUnixStyle() const                              { return unixStylePath (path); }
        String toWindowsStyle() const                           { return windowsStylePath (path); }

        String getFileName() const                              { return getFakeFile().getFileName(); }
        String getFileNameWithoutExtension() const              { return getFakeFile().getFileNameWithoutExtension(); }

        String getFileExtension() const                         { return getFakeFile().getFileExtension(); }
        bool hasFileExtension (StringRef extension) const       { return getFakeFile().hasFileExtension (extension); }
        bool isAbsolute() const                                 { return isAbsolutePath (path); }

        RelativePath withFileExtension (const String& extension) const
        {
            return RelativePath (path.upToLastOccurrenceOf (".", ! extension.startsWithChar ('.'), false) + extension, root);
        }

        RelativePath getParentDirectory() const
        {
            String p (path);
            if (path.endsWithChar ('/'))
                p = p.dropLastCharacters (1);

            return RelativePath (p.upToLastOccurrenceOf ("/", false, false), root);
        }

        RelativePath getChildFile (const String& subpath) const
        {
            if (isAbsolutePath (subpath))
                return RelativePath (subpath, root);

            String p (toUnixStyle());
            if (! p.endsWithChar ('/'))
                p << '/';

            return RelativePath (p + subpath, root);
        }

        RelativePath rebased (const File& originalRoot, const File& newRoot, const RootFolder newRootType) const
        {
            if (isAbsolute())
                return RelativePath (path, newRootType);

            return RelativePath (getRelativePathFrom (originalRoot.getChildFile (toUnixStyle()), newRoot), newRootType);
        }

    private:
        //==============================================================================
        String path;
        RootFolder root;

        File getFakeFile() const
        {
            const auto unixStylePath = toUnixStyle();
            const auto name = unixStylePath.substring (unixStylePath.lastIndexOfChar ('/') + 1);

            // This method gets called very often, so we'll cache this directory.
            static const File currentWorkingDirectory (File::getCurrentWorkingDirectory());
            return currentWorkingDirectory.getChildFile (name);
        }
    };

} // namespace juce::build_tools
