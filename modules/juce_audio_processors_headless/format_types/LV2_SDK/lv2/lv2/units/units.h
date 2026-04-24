/*
  Copyright 2012-2016 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#ifndef LV2_UNITS_H
#define LV2_UNITS_H

/**
   @defgroup units Units
   @ingroup lv2

   Units for LV2 values.

   See <http://lv2plug.in/ns/extensions/units> for details.

   @{
*/

// clang-format off

#define LV2_UNITS_URI    "http://lv2plug.in/ns/extensions/units"  ///< http://lv2plug.in/ns/extensions/units
#define LV2_UNITS_PREFIX LV2_UNITS_URI "#"                        ///< http://lv2plug.in/ns/extensions/units#

#define LV2_UNITS__Conversion       LV2_UNITS_PREFIX "Conversion"        ///< http://lv2plug.in/ns/extensions/units#Conversion
#define LV2_UNITS__Unit             LV2_UNITS_PREFIX "Unit"              ///< http://lv2plug.in/ns/extensions/units#Unit
#define LV2_UNITS__bar              LV2_UNITS_PREFIX "bar"               ///< http://lv2plug.in/ns/extensions/units#bar
#define LV2_UNITS__beat             LV2_UNITS_PREFIX "beat"              ///< http://lv2plug.in/ns/extensions/units#beat
#define LV2_UNITS__bpm              LV2_UNITS_PREFIX "bpm"               ///< http://lv2plug.in/ns/extensions/units#bpm
#define LV2_UNITS__cent             LV2_UNITS_PREFIX "cent"              ///< http://lv2plug.in/ns/extensions/units#cent
#define LV2_UNITS__cm               LV2_UNITS_PREFIX "cm"                ///< http://lv2plug.in/ns/extensions/units#cm
#define LV2_UNITS__coef             LV2_UNITS_PREFIX "coef"              ///< http://lv2plug.in/ns/extensions/units#coef
#define LV2_UNITS__conversion       LV2_UNITS_PREFIX "conversion"        ///< http://lv2plug.in/ns/extensions/units#conversion
#define LV2_UNITS__db               LV2_UNITS_PREFIX "db"                ///< http://lv2plug.in/ns/extensions/units#db
#define LV2_UNITS__degree           LV2_UNITS_PREFIX "degree"            ///< http://lv2plug.in/ns/extensions/units#degree
#define LV2_UNITS__frame            LV2_UNITS_PREFIX "frame"             ///< http://lv2plug.in/ns/extensions/units#frame
#define LV2_UNITS__hz               LV2_UNITS_PREFIX "hz"                ///< http://lv2plug.in/ns/extensions/units#hz
#define LV2_UNITS__inch             LV2_UNITS_PREFIX "inch"              ///< http://lv2plug.in/ns/extensions/units#inch
#define LV2_UNITS__khz              LV2_UNITS_PREFIX "khz"               ///< http://lv2plug.in/ns/extensions/units#khz
#define LV2_UNITS__km               LV2_UNITS_PREFIX "km"                ///< http://lv2plug.in/ns/extensions/units#km
#define LV2_UNITS__m                LV2_UNITS_PREFIX "m"                 ///< http://lv2plug.in/ns/extensions/units#m
#define LV2_UNITS__mhz              LV2_UNITS_PREFIX "mhz"               ///< http://lv2plug.in/ns/extensions/units#mhz
#define LV2_UNITS__midiNote         LV2_UNITS_PREFIX "midiNote"          ///< http://lv2plug.in/ns/extensions/units#midiNote
#define LV2_UNITS__mile             LV2_UNITS_PREFIX "mile"              ///< http://lv2plug.in/ns/extensions/units#mile
#define LV2_UNITS__min              LV2_UNITS_PREFIX "min"               ///< http://lv2plug.in/ns/extensions/units#min
#define LV2_UNITS__mm               LV2_UNITS_PREFIX "mm"                ///< http://lv2plug.in/ns/extensions/units#mm
#define LV2_UNITS__ms               LV2_UNITS_PREFIX "ms"                ///< http://lv2plug.in/ns/extensions/units#ms
#define LV2_UNITS__name             LV2_UNITS_PREFIX "name"              ///< http://lv2plug.in/ns/extensions/units#name
#define LV2_UNITS__oct              LV2_UNITS_PREFIX "oct"               ///< http://lv2plug.in/ns/extensions/units#oct
#define LV2_UNITS__pc               LV2_UNITS_PREFIX "pc"                ///< http://lv2plug.in/ns/extensions/units#pc
#define LV2_UNITS__prefixConversion LV2_UNITS_PREFIX "prefixConversion"  ///< http://lv2plug.in/ns/extensions/units#prefixConversion
#define LV2_UNITS__render           LV2_UNITS_PREFIX "render"            ///< http://lv2plug.in/ns/extensions/units#render
#define LV2_UNITS__s                LV2_UNITS_PREFIX "s"                 ///< http://lv2plug.in/ns/extensions/units#s
#define LV2_UNITS__semitone12TET    LV2_UNITS_PREFIX "semitone12TET"     ///< http://lv2plug.in/ns/extensions/units#semitone12TET
#define LV2_UNITS__symbol           LV2_UNITS_PREFIX "symbol"            ///< http://lv2plug.in/ns/extensions/units#symbol
#define LV2_UNITS__unit             LV2_UNITS_PREFIX "unit"              ///< http://lv2plug.in/ns/extensions/units#unit

// clang-format on

/**
   @}
*/

#endif /* LV2_UNITS_H */
