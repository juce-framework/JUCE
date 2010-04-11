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


//==============================================================================
int64 calculateStreamHashCode (InputStream& stream);
int64 calculateFileHashCode (const File& file);
bool areFilesIdentical (const File& file1, const File& file2);

bool overwriteFileWithNewDataIfDifferent (const File& file, const char* data, int numBytes);
bool overwriteFileWithNewDataIfDifferent (const File& file, const MemoryOutputStream& newData);
bool overwriteFileWithNewDataIfDifferent (const File& file, const String& newData);

bool containsAnyNonHiddenFiles (const File& folder);

//==============================================================================
// String::hashCode64 actually hit some dupes, so this is a more powerful version.
const int64 hashCode64 (const String& s);
const String randomHexString (Random& random, int numChars);
const String hexString8Digits (int value);

const String createAlphaNumericUID();
const String createGUID (const String& seed); // Turns a seed into a windows GUID

const String unixStylePath (const String& path);
const String windowsStylePath (const String& path);

bool shouldPathsBeRelative (String path1, String path2);

//==============================================================================
bool isJuceFolder (const File& folder);
const File findParentJuceFolder (const File& file);
const File findDefaultJuceFolder();

//==============================================================================
const String createIncludeStatement (const File& includeFile, const File& targetFile);
const String makeHeaderGuardName (const File& file);

const String replaceCEscapeChars (const String& s);

const String makeValidCppIdentifier (String s,
                                     const bool capitalise,
                                     const bool removeColons,
                                     const bool allowTemplates);

//==============================================================================
const String boolToCode (const bool b);
const String floatToCode (const float v);
const String doubleToCode (const double v);
const String colourToCode (const Colour& col);
const String justificationToCode (const Justification& justification);
const String castToFloat (const String& expression);

//==============================================================================
const String indentCode (const String& code, const int numSpaces);

int indexOfLineStartingWith (const StringArray& lines, const String& text, int startIndex);

//==============================================================================
class FileModificationDetector
{
public:
    FileModificationDetector (const File& file_)
        : file (file_)
    {
    }

    const File& getFile() const         { return file; }

    bool hasBeenModified() const
    {
        return fileModificationTime != file.getLastModificationTime()
                 && (fileSize != file.getSize()
                      || calculateFileHashCode (file) != fileHashCode);
    }

    void updateHash()
    {
        fileModificationTime = file.getLastModificationTime();
        fileSize = file.getSize();
        fileHashCode = calculateFileHashCode (file);
    }

private:
    File file;
    Time fileModificationTime;
    int64 fileHashCode, fileSize;
};


//==============================================================================
class RelativePosition
{
public:
    RelativePosition();
    explicit RelativePosition (const String& stringVersion);
    explicit RelativePosition (double absoluteDistanceFromOrigin);
    RelativePosition (double absoluteDistance, const String& source);
    RelativePosition (double relativeProportion, const String& pos1, const String& pos2);
    ~RelativePosition();

    class PositionFinder
    {
    public:
        virtual ~PositionFinder() {}
        virtual RelativePosition* findPosition (const String& name) = 0;
    };

    const String getName() const                        { return name; }
    void setName (const String& newName)                { name = newName; }

    double resolve (PositionFinder& positionFinder) const;
    void moveToAbsolute (double newPos, PositionFinder& positionFinder);

    const String toString (int decimalPlaces) const;

    static const char* parentOriginMarkerName;
    static const char* parentExtentMarkerName;

private:
    String name, nameOfSource1, nameOfSource2;
    double value;
    bool isRelative;

    double getPos1 (PositionFinder& positionFinder) const      { return getPosition (nameOfSource1, positionFinder); }
    double getPos2 (PositionFinder& positionFinder) const      { return getPosition (nameOfSource2, positionFinder); }
    double getPosition (const String& name, PositionFinder& positionFinder) const;
    static const String checkName (const String& name);
    static bool isOrigin (const String& name);
};

class RelativeRectangle
{
public:
    RelativeRectangle();
    explicit RelativeRectangle (const Rectangle<int>& rect);
    explicit RelativeRectangle (const String& stringVersion);

    const Rectangle<int> resolve (RelativePosition::PositionFinder& positionFinder) const;
    void moveToAbsolute (const Rectangle<int>& newPos, RelativePosition::PositionFinder& positionFinder);
    const String toString (int decimalPlaces) const;

    RelativePosition left, right, top, bottom;
};
