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
class PropertyPanelWithTooltips  : public Component,
                                   public Timer
{
public:
    PropertyPanelWithTooltips();
    ~PropertyPanelWithTooltips();

    PropertyPanel* getPanel() const        { return panel; }

    void paint (Graphics& g);
    void resized();
    void timerCallback();

private:
    PropertyPanel* panel;
    TextLayout layout;
    Component* lastComp;
    String lastTip;

    const String findTip (Component* c);
};

//==============================================================================
class FloatingLabelComponent    : public Component
{
public:
    FloatingLabelComponent();

    void remove();
    void update (Component* parent, const String& text, const Colour& textColour, int x, int y, bool toRight, bool below);
    void paint (Graphics& g);

private:
    Font font;
    Colour colour;
    GlyphArrangement glyphs;
};

//==============================================================================
static const double tickSizes[] = { 1.0, 2.0, 5.0,
                                    10.0, 20.0, 50.0,
                                    100.0, 200.0, 500.0, 1000.0 };

class TickIterator
{
public:
    TickIterator (const double startValue_, const double endValue_, const double valuePerPixel_,
                  int minPixelsPerTick, int minWidthForLabels)
        : startValue (startValue_),
          endValue (endValue_),
          valuePerPixel (valuePerPixel_)
    {
        tickLevelIndex  = findLevelIndexForValue (valuePerPixel * minPixelsPerTick);
        labelLevelIndex = findLevelIndexForValue (valuePerPixel * minWidthForLabels);

        tickPosition = pixelsToValue (-minWidthForLabels);
        tickPosition = snapValueDown (tickPosition, tickLevelIndex);
    }

    bool getNextTick (float& pixelX, float& tickLength, String& label)
    {
        const double tickUnits = tickSizes [tickLevelIndex];
        tickPosition += tickUnits;

        const int totalLevels = sizeof (tickSizes) / sizeof (*tickSizes);
        int highestIndex = tickLevelIndex;

        while (++highestIndex < totalLevels)
        {
            const double ticksAtThisLevel = tickPosition / tickSizes [highestIndex];

            if (fabs (ticksAtThisLevel - floor (ticksAtThisLevel + 0.5)) > 0.000001)
                break;
        }

        --highestIndex;

        if (highestIndex >= labelLevelIndex)
            label = getDescriptionOfValue (tickPosition, labelLevelIndex);
        else
            label = String::empty;

        tickLength = (highestIndex + 1 - tickLevelIndex) / (float) (totalLevels + 1 - tickLevelIndex);
        pixelX = valueToPixels (tickPosition);

        return tickPosition < endValue;
    }

private:
    double tickPosition;
    int tickLevelIndex, labelLevelIndex;
    const double startValue, endValue, valuePerPixel;

    int findLevelIndexForValue (const double value) const
    {
        int i;
        for (i = 0; i < sizeof (tickSizes) / sizeof (*tickSizes); ++i)
            if (tickSizes [i] >= value)
                break;

        return i;
    }

    double pixelsToValue (int pixels) const
    {
        return startValue + pixels * valuePerPixel;
    }

    float valueToPixels (double value) const
    {
        return (float) ((value - startValue) / valuePerPixel);
    }

    static double snapValueToNearest (const double t, const int valueLevelIndex)
    {
        const double unitsPerInterval = tickSizes [valueLevelIndex];
        return unitsPerInterval * floor (t / unitsPerInterval + 0.5);
    }

    static double snapValueDown (const double t, const int valueLevelIndex)
    {
        const double unitsPerInterval = tickSizes [valueLevelIndex];
        return unitsPerInterval * floor (t / unitsPerInterval);
    }

    static inline int roundDoubleToInt (const double value)
    {
        union { int asInt[2]; double asDouble; } n;
        n.asDouble = value + 6755399441055744.0;

    #if TARGET_RT_BIG_ENDIAN
        return n.asInt [1];
    #else
        return n.asInt [0];
    #endif
    }

    static const String getDescriptionOfValue (const double value, const int valueLevelIndex)
    {
        return String (roundToInt (value));
    }

    TickIterator (const TickIterator&);
    TickIterator& operator= (const TickIterator&);
};
