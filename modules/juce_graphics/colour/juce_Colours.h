/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
namespace Colours
{

    const Colour transparentBlack     { 0 };
    const Colour transparentWhite     { 0x00ffffff };

    const Colour aliceblue            { 0xfff0f8ff };
    const Colour antiquewhite         { 0xfffaebd7 };
    const Colour aqua                 { 0xff00ffff };
    const Colour aquamarine           { 0xff7fffd4 };
    const Colour azure                { 0xfff0ffff };
    const Colour beige                { 0xfff5f5dc };
    const Colour bisque               { 0xffffe4c4 };
    const Colour black                { 0xff000000 };
    const Colour blanchedalmond       { 0xffffebcd };
    const Colour blue                 { 0xff0000ff };
    const Colour blueviolet           { 0xff8a2be2 };
    const Colour brown                { 0xffa52a2a };
    const Colour burlywood            { 0xffdeb887 };
    const Colour cadetblue            { 0xff5f9ea0 };
    const Colour chartreuse           { 0xff7fff00 };
    const Colour chocolate            { 0xffd2691e };
    const Colour coral                { 0xffff7f50 };
    const Colour cornflowerblue       { 0xff6495ed };
    const Colour cornsilk             { 0xfffff8dc };
    const Colour crimson              { 0xffdc143c };
    const Colour cyan                 { 0xff00ffff };
    const Colour darkblue             { 0xff00008b };
    const Colour darkcyan             { 0xff008b8b };
    const Colour darkgoldenrod        { 0xffb8860b };
    const Colour darkgrey             { 0xff555555 };
    const Colour darkgreen            { 0xff006400 };
    const Colour darkkhaki            { 0xffbdb76b };
    const Colour darkmagenta          { 0xff8b008b };
    const Colour darkolivegreen       { 0xff556b2f };
    const Colour darkorange           { 0xffff8c00 };
    const Colour darkorchid           { 0xff9932cc };
    const Colour darkred              { 0xff8b0000 };
    const Colour darksalmon           { 0xffe9967a };
    const Colour darkseagreen         { 0xff8fbc8f };
    const Colour darkslateblue        { 0xff483d8b };
    const Colour darkslategrey        { 0xff2f4f4f };
    const Colour darkturquoise        { 0xff00ced1 };
    const Colour darkviolet           { 0xff9400d3 };
    const Colour deeppink             { 0xffff1493 };
    const Colour deepskyblue          { 0xff00bfff };
    const Colour dimgrey              { 0xff696969 };
    const Colour dodgerblue           { 0xff1e90ff };
    const Colour firebrick            { 0xffb22222 };
    const Colour floralwhite          { 0xfffffaf0 };
    const Colour forestgreen          { 0xff228b22 };
    const Colour fuchsia              { 0xffff00ff };
    const Colour gainsboro            { 0xffdcdcdc };
    const Colour ghostwhite           { 0xfff8f8ff };
    const Colour gold                 { 0xffffd700 };
    const Colour goldenrod            { 0xffdaa520 };
    const Colour grey                 { 0xff808080 };
    const Colour green                { 0xff008000 };
    const Colour greenyellow          { 0xffadff2f };
    const Colour honeydew             { 0xfff0fff0 };
    const Colour hotpink              { 0xffff69b4 };
    const Colour indianred            { 0xffcd5c5c };
    const Colour indigo               { 0xff4b0082 };
    const Colour ivory                { 0xfffffff0 };
    const Colour khaki                { 0xfff0e68c };
    const Colour lavender             { 0xffe6e6fa };
    const Colour lavenderblush        { 0xfffff0f5 };
    const Colour lawngreen            { 0xff7cfc00 };
    const Colour lemonchiffon         { 0xfffffacd };
    const Colour lightblue            { 0xffadd8e6 };
    const Colour lightcoral           { 0xfff08080 };
    const Colour lightcyan            { 0xffe0ffff };
    const Colour lightgoldenrodyellow { 0xfffafad2 };
    const Colour lightgreen           { 0xff90ee90 };
    const Colour lightgrey            { 0xffd3d3d3 };
    const Colour lightpink            { 0xffffb6c1 };
    const Colour lightsalmon          { 0xffffa07a };
    const Colour lightseagreen        { 0xff20b2aa };
    const Colour lightskyblue         { 0xff87cefa };
    const Colour lightslategrey       { 0xff778899 };
    const Colour lightsteelblue       { 0xffb0c4de };
    const Colour lightyellow          { 0xffffffe0 };
    const Colour lime                 { 0xff00ff00 };
    const Colour limegreen            { 0xff32cd32 };
    const Colour linen                { 0xfffaf0e6 };
    const Colour magenta              { 0xffff00ff };
    const Colour maroon               { 0xff800000 };
    const Colour mediumaquamarine     { 0xff66cdaa };
    const Colour mediumblue           { 0xff0000cd };
    const Colour mediumorchid         { 0xffba55d3 };
    const Colour mediumpurple         { 0xff9370db };
    const Colour mediumseagreen       { 0xff3cb371 };
    const Colour mediumslateblue      { 0xff7b68ee };
    const Colour mediumspringgreen    { 0xff00fa9a };
    const Colour mediumturquoise      { 0xff48d1cc };
    const Colour mediumvioletred      { 0xffc71585 };
    const Colour midnightblue         { 0xff191970 };
    const Colour mintcream            { 0xfff5fffa };
    const Colour mistyrose            { 0xffffe4e1 };
    const Colour moccasin             { 0xffffe4b5 };
    const Colour navajowhite          { 0xffffdead };
    const Colour navy                 { 0xff000080 };
    const Colour oldlace              { 0xfffdf5e6 };
    const Colour olive                { 0xff808000 };
    const Colour olivedrab            { 0xff6b8e23 };
    const Colour orange               { 0xffffa500 };
    const Colour orangered            { 0xffff4500 };
    const Colour orchid               { 0xffda70d6 };
    const Colour palegoldenrod        { 0xffeee8aa };
    const Colour palegreen            { 0xff98fb98 };
    const Colour paleturquoise        { 0xffafeeee };
    const Colour palevioletred        { 0xffdb7093 };
    const Colour papayawhip           { 0xffffefd5 };
    const Colour peachpuff            { 0xffffdab9 };
    const Colour peru                 { 0xffcd853f };
    const Colour pink                 { 0xffffc0cb };
    const Colour plum                 { 0xffdda0dd };
    const Colour powderblue           { 0xffb0e0e6 };
    const Colour purple               { 0xff800080 };
    const Colour rebeccapurple        { 0xff663399 };
    const Colour red                  { 0xffff0000 };
    const Colour rosybrown            { 0xffbc8f8f };
    const Colour royalblue            { 0xff4169e1 };
    const Colour saddlebrown          { 0xff8b4513 };
    const Colour salmon               { 0xfffa8072 };
    const Colour sandybrown           { 0xfff4a460 };
    const Colour seagreen             { 0xff2e8b57 };
    const Colour seashell             { 0xfffff5ee };
    const Colour sienna               { 0xffa0522d };
    const Colour silver               { 0xffc0c0c0 };
    const Colour skyblue              { 0xff87ceeb };
    const Colour slateblue            { 0xff6a5acd };
    const Colour slategrey            { 0xff708090 };
    const Colour snow                 { 0xfffffafa };
    const Colour springgreen          { 0xff00ff7f };
    const Colour steelblue            { 0xff4682b4 };
    const Colour tan                  { 0xffd2b48c };
    const Colour teal                 { 0xff008080 };
    const Colour thistle              { 0xffd8bfd8 };
    const Colour tomato               { 0xffff6347 };
    const Colour turquoise            { 0xff40e0d0 };
    const Colour violet               { 0xffee82ee };
    const Colour wheat                { 0xfff5deb3 };
    const Colour white                { 0xffffffff };
    const Colour whitesmoke           { 0xfff5f5f5 };
    const Colour yellow               { 0xffffff00 };
    const Colour yellowgreen          { 0xff9acd32 };

    /** Attempts to look up a string in the list of known colour names, and return
        the appropriate colour.

        A non-case-sensitive search is made of the list of predefined colours, and
        if a match is found, that colour is returned. If no match is found, the
        colour passed in as the defaultColour parameter is returned.
    */
    JUCE_API Colour findColourForName (const String& colourName,
                                       Colour defaultColour);
} // namespace Colours

} // namespace juce
