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

namespace juce
{

//==============================================================================
/**
    Contains a set of predefined named colours (mostly standard HTML colours)

    @see Colour

    @tags{Graphics}
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
    aliceblue,         antiquewhite,       aqua,                   aquamarine,
    azure,             beige,              bisque,                 blanchedalmond,
    blueviolet,        brown,              burlywood,              cadetblue,
    chartreuse,        chocolate,          coral,                  cornflowerblue,
    cornsilk,          crimson,            cyan,                   darkblue,
    darkcyan,          darkgoldenrod,      darkgrey,               darkgreen,
    darkkhaki,         darkmagenta,        darkolivegreen,         darkorange,
    darkorchid,        darkred,            darksalmon,             darkseagreen,
    darkslateblue,     darkslategrey,      darkturquoise,          darkviolet,
    deeppink,          deepskyblue,        dimgrey,                dodgerblue,
    firebrick,         floralwhite,        forestgreen,            fuchsia,
    gainsboro,         ghostwhite,         gold,                   goldenrod,
    greenyellow,       honeydew,           hotpink,                indianred,
    indigo,            ivory,              khaki,                  lavender,
    lavenderblush,     lawngreen,          lemonchiffon,           lightblue,
    lightcoral,        lightcyan,          lightgoldenrodyellow,   lightgreen,
    lightgrey,         lightpink,          lightsalmon,            lightseagreen,
    lightskyblue,      lightslategrey,     lightsteelblue,         lightyellow,
    lime,              limegreen,          linen,                  magenta,
    maroon,            mediumaquamarine,   mediumblue,             mediumorchid,
    mediumpurple,      mediumseagreen,     mediumslateblue,        mediumspringgreen,
    mediumturquoise,   mediumvioletred,    midnightblue,           mintcream,
    mistyrose,         moccasin,           navajowhite,            navy,
    oldlace,           olive,              olivedrab,              orange,
    orangered,         orchid,             palegoldenrod,          palegreen,
    paleturquoise,     palevioletred,      papayawhip,             peachpuff,
    peru,              pink,               plum,                   powderblue,
    purple,            rebeccapurple,      rosybrown,              royalblue,
    saddlebrown,       salmon,             sandybrown,             seagreen,
    seashell,          sienna,             silver,                 skyblue,
    slateblue,         slategrey,          snow,                   springgreen,
    steelblue,         tan,                teal,                   thistle,
    tomato,            turquoise,          violet,                 wheat,
    whitesmoke,        yellowgreen;

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

} // namespace juce
