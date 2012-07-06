/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "../jucer_Headers.h"
#include "jucer_StoredSettings.h"
#include "../Application/jucer_Application.h"

//==============================================================================
StoredSettings& getAppSettings()
{
    return JucerApplication::getApp()->settings;
}

PropertiesFile& getAppProperties()
{
    return getAppSettings().getProps();
}

static AppearanceSettings getDefaultScheme()
{
    CodeDocument doc;
    CPlusPlusCodeTokeniser tokeniser;
    CodeEditorComponent defaultComp (doc, &tokeniser);
    return AppearanceSettings (defaultComp);
}

//==============================================================================
StoredSettings::StoredSettings()
    : appearance (getDefaultScheme())
{
    reload();

    const File defaultSchemeFile (getSchemesFolder().getChildFile ("Default").withFileExtension (getSchemeFileSuffix()));

    if (! defaultSchemeFile.exists())
        appearance.writeToFile (defaultSchemeFile);
}

StoredSettings::~StoredSettings()
{
    flush();
}

PropertiesFile& StoredSettings::getProps()
{
    jassert (props != nullptr);
    return *props;
}

void StoredSettings::flush()
{
    if (props != nullptr)
    {
        {
            const ScopedPointer<XmlElement> xml (appearance.settings.createXml());
            props->setValue ("editorColours", xml);
        }

        props->setValue ("recentFiles", recentFiles.toString());

        props->removeValue ("keyMappings");

        if (commandManager != nullptr)
        {
            ScopedPointer <XmlElement> keys (commandManager->getKeyMappings()->createXml (true));

            if (keys != nullptr)
                props->setValue ("keyMappings", (XmlElement*) keys);
        }

        props->saveIfNeeded();
    }
}

void StoredSettings::reload()
{
    props = nullptr;

    {
        // These settings are used in defining the properties file's location.
        PropertiesFile::Options options;
        options.applicationName     = "Introjucer";
        options.folderName          = "Introjucer";
        options.filenameSuffix      = "settings";
        options.osxLibrarySubFolder = "Application Support";

        props = new PropertiesFile (options);

        // Because older versions of the introjucer saved their settings under a different
        // name, this code is an example of how to migrate your old settings files...
        if (! props->getFile().exists())
        {
            PropertiesFile::Options oldOptions;
            oldOptions.applicationName      = "Jucer2";
            oldOptions.filenameSuffix       = "settings";
            oldOptions.osxLibrarySubFolder  = "Preferences";

            PropertiesFile oldProps (oldOptions);

            if (oldProps.getFile().exists())
                props->addAllPropertiesFrom (oldProps);
        }
    }

    // recent files...
    recentFiles.restoreFromString (props->getValue ("recentFiles"));
    recentFiles.removeNonExistentFiles();

    const ScopedPointer<XmlElement> xml (props->getXmlValue ("editorColours"));
    if (xml != nullptr)
        appearance.readFromXML (*xml);

    loadSwatchColours();
}

Array<File> StoredSettings::getLastProjects() const
{
    StringArray s;
    s.addTokens (props->getValue ("lastProjects"), "|", "");

    Array<File> f;
    for (int i = 0; i < s.size(); ++i)
        f.add (File (s[i]));

    return f;
}

void StoredSettings::setLastProjects (const Array<File>& files)
{
    StringArray s;
    for (int i = 0; i < files.size(); ++i)
        s.add (files.getReference(i).getFullPathName());

    props->setValue ("lastProjects", s.joinIntoString ("|"));
}

const StringArray& StoredSettings::getFontNames()
{
    if (fontNames.size() == 0)
        fontNames = Font::findAllTypefaceNames();

    return fontNames;
}

//==============================================================================
void StoredSettings::loadSwatchColours()
{
    swatchColours.clear();

    #define COL(col)  Colours::col,

    const Colour colours[] =
    {
        #include "jucer_Colours.h"
        Colours::transparentBlack
    };

    #undef COL

    const int numSwatchColours = 24;

    for (int i = 0; i < numSwatchColours; ++i)
        swatchColours.add (Colour::fromString (props->getValue ("swatchColour" + String (i),
                                                                colours [2 + i].toString())));
}

int StoredSettings::ColourSelectorWithSwatches::getNumSwatches() const
{
    return getAppSettings().swatchColours.size();
}

Colour StoredSettings::ColourSelectorWithSwatches::getSwatchColour (int index) const
{
    return getAppSettings().swatchColours [index];
}

void StoredSettings::ColourSelectorWithSwatches::setSwatchColour (int index, const Colour& newColour) const
{
    getAppSettings().swatchColours.set (index, newColour);
}

File StoredSettings::getSchemesFolder()
{
    File f (getProps().getFile().getSiblingFile ("Colour Schemes"));
    f.createDirectory();
    return f;
}

//==============================================================================
const Icons& getIcons()
{
    return JucerApplication::getApp()->icons;
}

Icons::Icons()
{
    reload (Colours::white);
}

static Drawable* createDrawableFromSVGPath (const Colour& colour, const char* pathString)
{
    XmlElement svg ("svg");
    XmlElement* path = svg.createNewChildElement ("path");
    path->setAttribute ("fill", "#" + colour.toDisplayString (false));
    path->setAttribute ("d", pathString);

    return Drawable::createFromSVG (svg);
}

void Icons::reload (const Colour& /*backgroundColour*/)
{
    const Colour iconColour (Colours::darkgrey);

    /*  Some of the icon images used here are based on icons from this project:
        http://raphaeljs.com/icons
        They're MIT licensed - the licensing info is on the linked page.

        I thought they looked pretty stylish in this context, and it was also very
        convenient to be able to paste-in the shapes directly as SVG path strings!
    */
    drawables.add (folder   = createDrawableFromSVGPath (Colours::darkgrey,  "M 76.531972,116.65943 C 138.19864,91.877357 199.54658,66.389965 261.4126,42.049945 c 26.20551,4.17061 17.00846,35.348262 27.46812,51.842442 13.17191,44.087933 9.69881,51.512093 30.18158,93.124923 -7.79805,3.15935 -15.59597,6.3187 -23.39388,9.47805 C 269.9507,143.92299 270.68187,124.6488 255.14018,68.762725 208.6656,76.96787 149.63136,111.49835 96.876892,132.86616 c 20.096368,70.18096 40.229418,141.99553 81.756898,204.0512 15.24262,21.51974 46.21675,10.98988 56.02749,-9.54254 13.20038,-16.5246 12.12083,-76.13991 21.41808,-93.76216 8.17116,-9.97231 16.50664,-13.63709 27.69311,-19.48495 50.88575,-20.6198 101.77151,-41.23961 152.65739,-61.85942 16.97618,7.01928 -2.60481,67.74804 -8.54318,82.55545 -12.1959,28.12442 -28.64236,40.75479 -52.50229,58.90091 -59.50287,23.63196 -104.33289,55.33391 -164.43294,77.49302 -25.20592,5.49938 -48.76333,-11.78739 -59.77927,-31.99889 -33.49404,-49.05055 -49.87845,-106.05947 -67.959828,-161.26271 -5.38388,-17.37827 -10.5735,-34.85117 -13.98158,-52.68629 0.58247,-3.82935 3.52241,-7.1755 7.3012,-8.61035 z"));
    drawables.add (document = createDrawableFromSVGPath (Colours::yellow.darker(1.0f), "M23.024,5.673c-1.744-1.694-3.625-3.051-5.168-3.236c-0.084-0.012-0.171-0.019-0.263-0.021H7.438c-0.162,0-0.322,0.063-0.436,0.18C6.889,2.71,6.822,2.87,6.822,3.033v25.75c0,0.162,0.063,0.317,0.18,0.435c0.117,0.116,0.271,0.179,0.436,0.179h18.364c0.162,0,0.317-0.062,0.434-0.179c0.117-0.117,0.182-0.272,0.182-0.435V11.648C26.382,9.659,24.824,7.49,23.024,5.673zM22.157,6.545c0.805,0.786,1.529,1.676,2.069,2.534c-0.468-0.185-0.959-0.322-1.42-0.431c-1.015-0.228-2.008-0.32-2.625-0.357c0.003-0.133,0.004-0.283,0.004-0.446c0-0.869-0.055-2.108-0.356-3.2c-0.003-0.01-0.005-0.02-0.009-0.03C20.584,5.119,21.416,5.788,22.157,6.545zM25.184,28.164H8.052V3.646h9.542v0.002c0.416-0.025,0.775,0.386,1.05,1.326c0.25,0.895,0.313,2.062,0.312,2.871c0.002,0.593-0.027,0.991-0.027,0.991l-0.049,0.652l0.656,0.007c0.003,0,1.516,0.018,3,0.355c1.426,0.308,2.541,0.922,2.645,1.617c0.004,0.062,0.005,0.124,0.004,0.182V28.164z"));
    drawables.add (imageDoc = createDrawableFromSVGPath (Colours::darkgrey,  "M25.25,25.25H6.75V6.75h18.5V25.25zM11.25,14c1.426,0,2.583-1.157,2.583-2.583c0-1.427-1.157-2.583-2.583-2.583c-1.427,0-2.583,1.157-2.583,2.583C8.667,12.843,9.823,14,11.25,14zM24.251,16.25l-4.917-4.917l-6.917,6.917L10.5,16.333l-2.752,2.752v5.165h16.503V16.25z"));
    drawables.add (config   = createDrawableFromSVGPath (Colours::darkgreen, "M17.41,20.395l-0.778-2.723c0.228-0.2,0.442-0.414,0.644-0.643l2.721,0.778c0.287-0.418,0.534-0.862,0.755-1.323l-2.025-1.96c0.097-0.288,0.181-0.581,0.241-0.883l2.729-0.684c0.02-0.252,0.039-0.505,0.039-0.763s-0.02-0.51-0.039-0.762l-2.729-0.684c-0.061-0.302-0.145-0.595-0.241-0.883l2.026-1.96c-0.222-0.46-0.469-0.905-0.756-1.323l-2.721,0.777c-0.201-0.228-0.416-0.442-0.644-0.643l0.778-2.722c-0.418-0.286-0.863-0.534-1.324-0.755l-1.96,2.026c-0.287-0.097-0.581-0.18-0.883-0.241l-0.683-2.73c-0.253-0.019-0.505-0.039-0.763-0.039s-0.51,0.02-0.762,0.039l-0.684,2.73c-0.302,0.061-0.595,0.144-0.883,0.241l-1.96-2.026C7.048,3.463,6.604,3.71,6.186,3.997l0.778,2.722C6.736,6.919,6.521,7.134,6.321,7.361L3.599,6.583C3.312,7.001,3.065,7.446,2.844,7.907l2.026,1.96c-0.096,0.288-0.18,0.581-0.241,0.883l-2.73,0.684c-0.019,0.252-0.039,0.505-0.039,0.762s0.02,0.51,0.039,0.763l2.73,0.684c0.061,0.302,0.145,0.595,0.241,0.883l-2.026,1.96c0.221,0.46,0.468,0.905,0.755,1.323l2.722-0.778c0.2,0.229,0.415,0.442,0.643,0.643l-0.778,2.723c0.418,0.286,0.863,0.533,1.323,0.755l1.96-2.026c0.288,0.097,0.581,0.181,0.883,0.241l0.684,2.729c0.252,0.02,0.505,0.039,0.763,0.039s0.51-0.02,0.763-0.039l0.683-2.729c0.302-0.061,0.596-0.145,0.883-0.241l1.96,2.026C16.547,20.928,16.992,20.681,17.41,20.395zM11.798,15.594c-1.877,0-3.399-1.522-3.399-3.399s1.522-3.398,3.399-3.398s3.398,1.521,3.398,3.398S13.675,15.594,11.798,15.594zM27.29,22.699c0.019-0.547-0.06-1.104-0.23-1.654l1.244-1.773c-0.188-0.35-0.4-0.682-0.641-0.984l-2.122,0.445c-0.428-0.364-0.915-0.648-1.436-0.851l-0.611-2.079c-0.386-0.068-0.777-0.105-1.173-0.106l-0.974,1.936c-0.279,0.054-0.558,0.128-0.832,0.233c-0.257,0.098-0.497,0.22-0.727,0.353L17.782,17.4c-0.297,0.262-0.568,0.545-0.813,0.852l0.907,1.968c-0.259,0.495-0.437,1.028-0.519,1.585l-1.891,1.06c0.019,0.388,0.076,0.776,0.164,1.165l2.104,0.519c0.231,0.524,0.541,0.993,0.916,1.393l-0.352,2.138c0.32,0.23,0.66,0.428,1.013,0.6l1.715-1.32c0.536,0.141,1.097,0.195,1.662,0.15l1.452,1.607c0.2-0.057,0.399-0.118,0.596-0.193c0.175-0.066,0.34-0.144,0.505-0.223l0.037-2.165c0.455-0.339,0.843-0.747,1.152-1.206l2.161-0.134c0.152-0.359,0.279-0.732,0.368-1.115L27.29,22.699zM23.127,24.706c-1.201,0.458-2.545-0.144-3.004-1.345s0.143-2.546,1.344-3.005c1.201-0.458,2.547,0.144,3.006,1.345C24.931,22.902,24.328,24.247,23.127,24.706z"));
    drawables.add (graph    = createDrawableFromSVGPath (Colours::darkred,   "M6.812,17.202l7.396-3.665v-2.164h-0.834c-0.414,0-0.808-0.084-1.167-0.237v1.159l-7.396,3.667v2.912h2V17.202zM26.561,18.875v-2.913l-7.396-3.666v-1.158c-0.358,0.152-0.753,0.236-1.166,0.236h-0.832l-0.001,2.164l7.396,3.666v1.672H26.561zM16.688,18.875v-7.501h-2v7.501H16.688zM27.875,19.875H23.25c-1.104,0-2,0.896-2,2V26.5c0,1.104,0.896,2,2,2h4.625c1.104,0,2-0.896,2-2v-4.625C29.875,20.771,28.979,19.875,27.875,19.875zM8.125,19.875H3.5c-1.104,0-2,0.896-2,2V26.5c0,1.104,0.896,2,2,2h4.625c1.104,0,2-0.896,2-2v-4.625C10.125,20.771,9.229,19.875,8.125,19.875zM13.375,10.375H18c1.104,0,2-0.896,2-2V3.75c0-1.104-0.896-2-2-2h-4.625c-1.104,0-2,0.896-2,2v4.625C11.375,9.479,12.271,10.375,13.375,10.375zM18,19.875h-4.625c-1.104,0-2,0.896-2,2V26.5c0,1.104,0.896,2,2,2H18c1.104,0,2-0.896,2-2v-4.625C20,20.771,19.104,19.875,18,19.875z"));
    drawables.add (exporter = createDrawableFromSVGPath (Colours::darkgrey,  "M15.067,2.25c-5.979,0-11.035,3.91-12.778,9.309h3.213c1.602-3.705,5.271-6.301,9.565-6.309c5.764,0.01,10.428,4.674,10.437,10.437c-0.009,5.764-4.673,10.428-10.437,10.438c-4.294-0.007-7.964-2.605-9.566-6.311H2.289c1.744,5.399,6.799,9.31,12.779,9.312c7.419-0.002,13.437-6.016,13.438-13.438C28.504,8.265,22.486,2.252,15.067,2.25zM10.918,19.813l7.15-4.126l-7.15-4.129v2.297H-0.057v3.661h10.975V19.813z"));
    drawables.add (jigsaw   = createDrawableFromSVGPath (Colours::darkgrey,  "M3.739,13.619c0,0,3.516-4.669,5.592-3.642c2.077,1.027-0.414,2.795,1.598,3.719c2.011,0.924,5.048-0.229,4.376-2.899c-0.672-2.67-1.866-0.776-2.798-2.208c-0.934-1.432,4.586-4.59,4.586-4.59s3.361,6.651,4.316,4.911c1.157-2.105,3.193-4.265,5.305-1.025c0,0,1.814,2.412,0.246,3.434s-2.917,0.443-3.506,1.553c-0.586,1.112,3.784,4.093,3.784,4.093s-2.987,4.81-4.926,3.548c-1.939-1.262,0.356-3.364-2.599-3.989c-1.288-0.23-3.438,0.538-3.818,2.34c-0.13,2.709,1.604,2.016,2.797,3.475c1.191,1.457-4.484,4.522-4.484,4.522s-1.584-3.923-3.811-4.657c-2.227-0.735-0.893,2.135-2.917,2.531c-2.024,0.396-4.816-2.399-3.46-4.789c1.358-2.391,3.275-0.044,3.441-1.951C7.629,16.087,3.739,13.619,3.739,13.619z"));
    drawables.add (info     = createDrawableFromSVGPath (Colours::yellow.darker(1.0f), "M16,1.466C7.973,1.466,1.466,7.973,1.466,16c0,8.027,6.507,14.534,14.534,14.534c8.027,0,14.534-6.507,14.534-14.534C30.534,7.973,24.027,1.466,16,1.466z M14.757,8h2.42v2.574h-2.42V8z M18.762,23.622H16.1c-1.034,0-1.475-0.44-1.475-1.496v-6.865c0-0.33-0.176-0.484-0.484-0.484h-0.88V12.4h2.662c1.035,0,1.474,0.462,1.474,1.496v6.887c0,0.309,0.176,0.484,0.484,0.484h0.88V23.622z"));
    drawables.add (warning  = createDrawableFromSVGPath (Colours::darkred,   "M29.225,23.567l-3.778-6.542c-1.139-1.972-3.002-5.2-4.141-7.172l-3.778-6.542c-1.14-1.973-3.003-1.973-4.142,0L9.609,9.853c-1.139,1.972-3.003,5.201-4.142,7.172L1.69,23.567c-1.139,1.974-0.207,3.587,2.071,3.587h23.391C29.432,27.154,30.363,25.541,29.225,23.567zM16.536,24.58h-2.241v-2.151h2.241V24.58zM16.428,20.844h-2.023l-0.201-9.204h2.407L16.428,20.844z"));
    drawables.add (bug      = createDrawableFromSVGPath (Colours::darkgrey,  "M28.589,10.903l-5.828,1.612c-0.534-1.419-1.338-2.649-2.311-3.628l3.082-5.44c0.271-0.48,0.104-1.092-0.38-1.365c-0.479-0.271-1.09-0.102-1.36,0.377l-2.924,5.162c-0.604-0.383-1.24-0.689-1.9-0.896c-0.416-1.437-1.652-2.411-3.058-2.562c-0.001-0.004-0.002-0.008-0.003-0.012c-0.061-0.242-0.093-0.46-0.098-0.65c-0.005-0.189,0.012-0.351,0.046-0.479c0.037-0.13,0.079-0.235,0.125-0.317c0.146-0.26,0.34-0.43,0.577-0.509c0.023,0.281,0.142,0.482,0.352,0.601c0.155,0.088,0.336,0.115,0.546,0.086c0.211-0.031,0.376-0.152,0.496-0.363c0.105-0.186,0.127-0.389,0.064-0.607c-0.064-0.219-0.203-0.388-0.414-0.507c-0.154-0.087-0.314-0.131-0.482-0.129c-0.167,0.001-0.327,0.034-0.481,0.097c-0.153,0.063-0.296,0.16-0.429,0.289c-0.132,0.129-0.241,0.271-0.33,0.426c-0.132,0.234-0.216,0.496-0.25,0.783c-0.033,0.286-0.037,0.565-0.009,0.84c0.017,0.16,0.061,0.301,0.094,0.449c-0.375-0.021-0.758,0.002-1.14,0.108c-0.482,0.133-0.913,0.36-1.28,0.653c-0.052-0.172-0.098-0.344-0.18-0.518c-0.116-0.249-0.263-0.486-0.438-0.716c-0.178-0.229-0.384-0.41-0.618-0.543C9.904,3.059,9.737,2.994,9.557,2.951c-0.18-0.043-0.352-0.052-0.516-0.027s-0.318,0.08-0.463,0.164C8.432,3.172,8.318,3.293,8.23,3.445C8.111,3.656,8.08,3.873,8.136,4.092c0.058,0.221,0.181,0.384,0.367,0.49c0.21,0.119,0.415,0.138,0.611,0.056C9.31,4.556,9.451,4.439,9.539,4.283c0.119-0.21,0.118-0.443-0.007-0.695c0.244-0.055,0.497-0.008,0.757,0.141c0.081,0.045,0.171,0.115,0.27,0.208c0.097,0.092,0.193,0.222,0.286,0.388c0.094,0.166,0.179,0.368,0.251,0.608c0.013,0.044,0.023,0.098,0.035,0.146c-0.911,0.828-1.357,2.088-1.098,3.357c-0.582,0.584-1.072,1.27-1.457,2.035l-5.16-2.926c-0.48-0.271-1.092-0.102-1.364,0.377C1.781,8.404,1.95,9.016,2.43,9.289l5.441,3.082c-0.331,1.34-0.387,2.807-0.117,4.297l-5.828,1.613c-0.534,0.147-0.846,0.699-0.698,1.231c0.147,0.53,0.697,0.843,1.231,0.694l5.879-1.627c0.503,1.057,1.363,2.28,2.371,3.443l-3.194,5.639c-0.272,0.481-0.104,1.092,0.378,1.363c0.239,0.137,0.512,0.162,0.758,0.094c0.248-0.068,0.469-0.229,0.604-0.471l2.895-5.109c2.7,2.594,5.684,4.123,5.778,1.053c1.598,2.56,3.451-0.338,4.502-3.976l5.203,2.947c0.24,0.138,0.514,0.162,0.762,0.094c0.246-0.067,0.467-0.229,0.603-0.471c0.272-0.479,0.104-1.091-0.377-1.362l-5.701-3.229c0.291-1.505,0.422-2.983,0.319-4.138l5.886-1.627c0.53-0.147,0.847-0.697,0.696-1.229C29.673,11.068,29.121,10.756,28.589,10.903z"));


    DrawableImage* juceImage = new DrawableImage();
    juceImage->setImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));
    drawables.add (juceLogo = juceImage);
}
