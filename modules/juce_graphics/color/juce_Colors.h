/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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
    Contains a set of predefined named colors (mostly standard HTML colors)

    @see Color

    @tags{Graphics}
*/
class Colors
{
public:
    static JUCE_API const Color

    //==============================================================================
    transparentBlack,   /**< ARGB = 0x00000000 */
    transparentWhite,   /**< ARGB = 0x00ffffff */

    //==============================================================================
    black,              /**< ARGB = 0xff000000 */
    white,              /**< ARGB = 0xffffffff */
    blue,               /**< ARGB = 0xff0000ff */
    gray,               /**< ARGB = 0xff808080 */
    green,              /**< ARGB = 0xff008000 */
    red,                /**< ARGB = 0xffff0000 */
    yellow,             /**< ARGB = 0xffffff00 */

    //==============================================================================
    aliceblue,         antiquewhite,       aqua,                   aquamarine,
    azure,             beige,              bisque,                 blanchedalmond,
    blueviolet,        brown,              burlywood,              cadetblue,
    chartreuse,        chocolate,          coral,                  cornflowerblue,
    cornsilk,          crimson,            cyan,                   darkblue,
    darkcyan,          darkgoldenrod,      darkgray,               darkgreen,
    darkkhaki,         darkmagenta,        darkolivegreen,         darkorange,
    darkorchid,        darkred,            darksalmon,             darkseagreen,
    darkslateblue,     darkslategray,      darkturquoise,          darkviolet,
    deeppink,          deepskyblue,        dimgray,                dodgerblue,
    firebrick,         floralwhite,        forestgreen,            fuchsia,
    gainsboro,         ghostwhite,         gold,                   goldenrod,
    greenyellow,       honeydew,           hotpink,                indianred,
    indigo,            ivory,              khaki,                  lavender,
    lavenderblush,     lawngreen,          lemonchiffon,           lightblue,
    lightcoral,        lightcyan,          lightgoldenrodyellow,   lightgreen,
    lightgray,         lightpink,          lightsalmon,            lightseagreen,
    lightskyblue,      lightslategray,     lightsteelblue,         lightyellow,
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
    slateblue,         slategray,          snow,                   springgreen,
    steelblue,         tan,                teal,                   thistle,
    tomato,            turquoise,          violet,                 wheat,
    whitesmoke,        yellowgreen;

    /** Attempts to look up a string in the list of known color names, and return
        the appropriate color.

        A non-case-sensitive search is made of the list of predefined colors, and
        if a match is found, that color is returned. If no match is found, the
        color passed in as the defaultColor parameter is returned.
    */
    static JUCE_API Color findColorForName (const String& colorName,
                                              Color defaultColor);

private:
    //==============================================================================
    // this isn't a class you should ever instantiate - it's just here for the
    // static values in it.
    Colors();

    JUCE_DECLARE_NON_COPYABLE (Colors)
};

} // namespace juce
