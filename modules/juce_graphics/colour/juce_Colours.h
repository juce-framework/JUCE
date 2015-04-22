/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_COLOURS_H_INCLUDED
#define JUCE_COLOURS_H_INCLUDED


//==============================================================================
/**
    Contains a set of predefined named colours (mostly standard HTML colours)

    @see Colour, Colours::greyLevel
*/
class Colours
{
public:
    static JUCE_API const Colour

    //==============================================================================
    transparentBlack,   /**< ARGB = 0x00000000 */
    transparentWhite,   /**< ARGB = 0x00ffffff */

    //==============================================================================
    black,              /**< ARGB = 0xff000000 */
    white,              /**< ARGB = 0xffffffff */
    blue,               /**< ARGB = 0xff0000ff */
    grey,               /**< ARGB = 0xff808080 */
    green,              /**< ARGB = 0xff008000 */
    red,                /**< ARGB = 0xffff0000 */
    yellow,             /**< ARGB = 0xffffff00 */

    //==============================================================================
    aliceblue,              antiquewhite,       aqua,               aquamarine,
    azure,                  beige,              bisque,             blanchedalmond,
    blueviolet,             brown,              burlywood,          cadetblue,
    chartreuse,             chocolate,          coral,              cornflowerblue,
    cornsilk,               crimson,            cyan,               darkblue,
    darkcyan,               darkgoldenrod,      darkgrey,           darkgreen,
    darkkhaki,              darkmagenta,        darkolivegreen,     darkorange,
    darkorchid,             darkred,            darksalmon,         darkseagreen,
    darkslateblue,          darkslategrey,      darkturquoise,      darkviolet,
    deeppink,               deepskyblue,        dimgrey,            dodgerblue,
    firebrick,              floralwhite,        forestgreen,        fuchsia,
    gainsboro,              gold,               goldenrod,          greenyellow,
    honeydew,               hotpink,            indianred,          indigo,
    ivory,                  khaki,              lavender,           lavenderblush,
    lemonchiffon,           lightblue,          lightcoral,         lightcyan,
    lightgoldenrodyellow,   lightgreen,         lightgrey,          lightpink,
    lightsalmon,            lightseagreen,      lightskyblue,       lightslategrey,
    lightsteelblue,         lightyellow,        lime,               limegreen,
    linen,                  magenta,            maroon,             mediumaquamarine,
    mediumblue,             mediumorchid,       mediumpurple,       mediumseagreen,
    mediumslateblue,        mediumspringgreen,  mediumturquoise,    mediumvioletred,
    midnightblue,           mintcream,          mistyrose,          navajowhite,
    navy,                   oldlace,            olive,              olivedrab,
    orange,                 orangered,          orchid,             palegoldenrod,
    palegreen,              paleturquoise,      palevioletred,      papayawhip,
    peachpuff,              peru,               pink,               plum,
    powderblue,             purple,             rosybrown,          royalblue,
    saddlebrown,            salmon,             sandybrown,         seagreen,
    seashell,               sienna,             silver,             skyblue,
    slateblue,              slategrey,          snow,               springgreen,
    steelblue,              tan,                teal,               thistle,
    tomato,                 turquoise,          violet,             wheat,
    whitesmoke,             yellowgreen;

    /** Attempts to look up a string in the list of known colour names, and return
        the appropriate colour.

        A non-case-sensitive search is made of the list of predefined colours, and
        if a match is found, that colour is returned. If no match is found, the
        colour passed in as the defaultColour parameter is returned.
    */
    static JUCE_API Colour findColourForName (const String& colourName,
                                              Colour defaultColour);

private:
    //==============================================================================
    // this isn't a class you should ever instantiate - it's just here for the
    // static values in it.
    Colours();

    JUCE_DECLARE_NON_COPYABLE (Colours)
};

#endif   // JUCE_COLOURS_H_INCLUDED
