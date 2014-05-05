/***************************************************************************/
/*                                                                         */
/*  FreeTypeAmalgam.h                                                      */
/*                                                                         */
/*  Copyright 2003-2007, 2011 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifdef _MSC_VER
#pragma push_macro("_CRT_SECURE_NO_WARNINGS")
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif


/*** Start of inlined file: ft2build.h ***/
/*                                                                         */
/*  ft2build.h                                                             */
/*                                                                         */
/*    FreeType 2 build and setup macros.                                   */
/*    (Generic version)                                                    */
/*                                                                         */
/*  Copyright 1996-2001, 2006 by                                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file corresponds to the default `ft2build.h' file for            */
  /* FreeType 2.  It uses the `freetype' include root.                     */
  /*                                                                       */
  /* Note that specific platforms might use a different configuration.     */
  /* See builds/unix/ft2unix.h for an example.                             */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FT2_BUILD_GENERIC_H__
#define __FT2_BUILD_GENERIC_H__


/*** Start of inlined file: ftheader.h ***/
/*                                                                         */
/*  ftheader.h                                                             */
/*                                                                         */
/*    Build macros of the FreeType 2 library.                              */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010 by */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FT_HEADER_H__
#define __FT_HEADER_H__

  /*@***********************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_BEGIN_HEADER                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro is used in association with @FT_END_HEADER in header    */
  /*    files to ensure that the declarations within are properly          */
  /*    encapsulated in an `extern "C" { .. }' block when included from a  */
  /*    C++ compiler.                                                      */
  /*                                                                       */
#ifdef __cplusplus
#define FT_BEGIN_HEADER  extern "C" {
#else
#define FT_BEGIN_HEADER  /* nothing */
#endif

  /*@***********************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_END_HEADER                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro is used in association with @FT_BEGIN_HEADER in header  */
  /*    files to ensure that the declarations within are properly          */
  /*    encapsulated in an `extern "C" { .. }' block when included from a  */
  /*    C++ compiler.                                                      */
  /*                                                                       */
#ifdef __cplusplus
#define FT_END_HEADER  }
#else
#define FT_END_HEADER  /* nothing */
#endif

  /*************************************************************************/
  /*                                                                       */
  /* Aliases for the FreeType 2 public and configuration files.            */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    header_file_macros                                                 */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Header File Macros                                                 */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Macro definitions used to #include specific header files.          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The following macros are defined to the name of specific           */
  /*    FreeType~2 header files.  They can be used directly in #include    */
  /*    statements as in:                                                  */
  /*                                                                       */
  /*    {                                                                  */
  /*      #include FT_FREETYPE_H                                           */
  /*      #include FT_MULTIPLE_MASTERS_H                                   */
  /*      #include FT_GLYPH_H                                              */
  /*    }                                                                  */
  /*                                                                       */
  /*    There are several reasons why we are now using macros to name      */
  /*    public header files.  The first one is that such macros are not    */
  /*    limited to the infamous 8.3~naming rule required by DOS (and       */
  /*    `FT_MULTIPLE_MASTERS_H' is a lot more meaningful than `ftmm.h').   */
  /*                                                                       */
  /*    The second reason is that it allows for more flexibility in the    */
  /*    way FreeType~2 is installed on a given system.                     */
  /*                                                                       */
  /*************************************************************************/

  /* configuration files */

  /*************************************************************************
   *
   * @macro:
   *   FT_CONFIG_CONFIG_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing
   *   FreeType~2 configuration data.
   *
   */
#ifndef FT_CONFIG_CONFIG_H
#define FT_CONFIG_CONFIG_H  <freetype/config/ftconfig.h>
#endif

  /*************************************************************************
   *
   * @macro:
   *   FT_CONFIG_STANDARD_LIBRARY_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing
   *   FreeType~2 interface to the standard C library functions.
   *
   */
#ifndef FT_CONFIG_STANDARD_LIBRARY_H
#define FT_CONFIG_STANDARD_LIBRARY_H  <freetype/config/ftstdlib.h>
#endif

  /*************************************************************************
   *
   * @macro:
   *   FT_CONFIG_OPTIONS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing
   *   FreeType~2 project-specific configuration options.
   *
   */
#ifndef FT_CONFIG_OPTIONS_H
#define FT_CONFIG_OPTIONS_H  <freetype/config/ftoption.h>
#endif

  /*************************************************************************
   *
   * @macro:
   *   FT_CONFIG_MODULES_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   list of FreeType~2 modules that are statically linked to new library
   *   instances in @FT_Init_FreeType.
   *
   */
#ifndef FT_CONFIG_MODULES_H
#define FT_CONFIG_MODULES_H  <freetype/config/ftmodule.h>
#endif

  /* */

  /* public headers */

  /*************************************************************************
   *
   * @macro:
   *   FT_FREETYPE_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   base FreeType~2 API.
   *
   */
#define FT_FREETYPE_H  <freetype/freetype.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_ERRORS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   list of FreeType~2 error codes (and messages).
   *
   *   It is included by @FT_FREETYPE_H.
   *
   */
#define FT_ERRORS_H  <freetype/fterrors.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_MODULE_ERRORS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   list of FreeType~2 module error offsets (and messages).
   *
   */
#define FT_MODULE_ERRORS_H  <freetype/ftmoderr.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_SYSTEM_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 interface to low-level operations (i.e., memory management
   *   and stream i/o).
   *
   *   It is included by @FT_FREETYPE_H.
   *
   */
#define FT_SYSTEM_H  <freetype/ftsystem.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_IMAGE_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing type
   *   definitions related to glyph images (i.e., bitmaps, outlines,
   *   scan-converter parameters).
   *
   *   It is included by @FT_FREETYPE_H.
   *
   */
#define FT_IMAGE_H  <freetype/ftimage.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_TYPES_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   basic data types defined by FreeType~2.
   *
   *   It is included by @FT_FREETYPE_H.
   *
   */
#define FT_TYPES_H  <freetype/fttypes.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_LIST_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   list management API of FreeType~2.
   *
   *   (Most applications will never need to include this file.)
   *
   */
#define FT_LIST_H  <freetype/ftlist.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_OUTLINE_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   scalable outline management API of FreeType~2.
   *
   */
#define FT_OUTLINE_H  <freetype/ftoutln.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_SIZES_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   API which manages multiple @FT_Size objects per face.
   *
   */
#define FT_SIZES_H  <freetype/ftsizes.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_MODULE_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   module management API of FreeType~2.
   *
   */
#define FT_MODULE_H  <freetype/ftmodapi.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_RENDER_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   renderer module management API of FreeType~2.
   *
   */
#define FT_RENDER_H  <freetype/ftrender.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_TYPE1_TABLES_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   types and API specific to the Type~1 format.
   *
   */
#define FT_TYPE1_TABLES_H  <freetype/t1tables.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_TRUETYPE_IDS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   enumeration values which identify name strings, languages, encodings,
   *   etc.  This file really contains a _large_ set of constant macro
   *   definitions, taken from the TrueType and OpenType specifications.
   *
   */
#define FT_TRUETYPE_IDS_H  <freetype/ttnameid.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_TRUETYPE_TABLES_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   types and API specific to the TrueType (as well as OpenType) format.
   *
   */
#define FT_TRUETYPE_TABLES_H  <freetype/tttables.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_TRUETYPE_TAGS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   definitions of TrueType four-byte `tags' which identify blocks in
   *   SFNT-based font formats (i.e., TrueType and OpenType).
   *
   */
#define FT_TRUETYPE_TAGS_H  <freetype/tttags.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_BDF_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   definitions of an API which accesses BDF-specific strings from a
   *   face.
   *
   */
#define FT_BDF_H  <freetype/ftbdf.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_CID_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   definitions of an API which access CID font information from a
   *   face.
   *
   */
#define FT_CID_H  <freetype/ftcid.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_GZIP_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   definitions of an API which supports gzip-compressed files.
   *
   */
#define FT_GZIP_H  <freetype/ftgzip.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_LZW_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   definitions of an API which supports LZW-compressed files.
   *
   */
#define FT_LZW_H  <freetype/ftlzw.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_BZIP2_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   definitions of an API which supports bzip2-compressed files.
   *
   */
#define FT_BZIP2_H  <freetype/ftbzip2.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_WINFONTS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   definitions of an API which supports Windows FNT files.
   *
   */
#define FT_WINFONTS_H   <freetype/ftwinfnt.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_GLYPH_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   API of the optional glyph management component.
   *
   */
#define FT_GLYPH_H  <freetype/ftglyph.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_BITMAP_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   API of the optional bitmap conversion component.
   *
   */
#define FT_BITMAP_H  <freetype/ftbitmap.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_BBOX_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   API of the optional exact bounding box computation routines.
   *
   */
#define FT_BBOX_H  <freetype/ftbbox.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_CACHE_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   API of the optional FreeType~2 cache sub-system.
   *
   */
#define FT_CACHE_H  <freetype/ftcache.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_CACHE_IMAGE_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   `glyph image' API of the FreeType~2 cache sub-system.
   *
   *   It is used to define a cache for @FT_Glyph elements.  You can also
   *   use the API defined in @FT_CACHE_SMALL_BITMAPS_H if you only need to
   *   store small glyph bitmaps, as it will use less memory.
   *
   *   This macro is deprecated.  Simply include @FT_CACHE_H to have all
   *   glyph image-related cache declarations.
   *
   */
#define FT_CACHE_IMAGE_H  FT_CACHE_H

  /*************************************************************************
   *
   * @macro:
   *   FT_CACHE_SMALL_BITMAPS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   `small bitmaps' API of the FreeType~2 cache sub-system.
   *
   *   It is used to define a cache for small glyph bitmaps in a relatively
   *   memory-efficient way.  You can also use the API defined in
   *   @FT_CACHE_IMAGE_H if you want to cache arbitrary glyph images,
   *   including scalable outlines.
   *
   *   This macro is deprecated.  Simply include @FT_CACHE_H to have all
   *   small bitmaps-related cache declarations.
   *
   */
#define FT_CACHE_SMALL_BITMAPS_H  FT_CACHE_H

  /*************************************************************************
   *
   * @macro:
   *   FT_CACHE_CHARMAP_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   `charmap' API of the FreeType~2 cache sub-system.
   *
   *   This macro is deprecated.  Simply include @FT_CACHE_H to have all
   *   charmap-based cache declarations.
   *
   */
#define FT_CACHE_CHARMAP_H  FT_CACHE_H

  /*************************************************************************
   *
   * @macro:
   *   FT_MAC_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   Macintosh-specific FreeType~2 API.  The latter is used to access
   *   fonts embedded in resource forks.
   *
   *   This header file must be explicitly included by client applications
   *   compiled on the Mac (note that the base API still works though).
   *
   */
#define FT_MAC_H  <freetype/ftmac.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_MULTIPLE_MASTERS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   optional multiple-masters management API of FreeType~2.
   *
   */
#define FT_MULTIPLE_MASTERS_H  <freetype/ftmm.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_SFNT_NAMES_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   optional FreeType~2 API which accesses embedded `name' strings in
   *   SFNT-based font formats (i.e., TrueType and OpenType).
   *
   */
#define FT_SFNT_NAMES_H  <freetype/ftsnames.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_OPENTYPE_VALIDATE_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   optional FreeType~2 API which validates OpenType tables (BASE, GDEF,
   *   GPOS, GSUB, JSTF).
   *
   */
#define FT_OPENTYPE_VALIDATE_H  <freetype/ftotval.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_GX_VALIDATE_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   optional FreeType~2 API which validates TrueTypeGX/AAT tables (feat,
   *   mort, morx, bsln, just, kern, opbd, trak, prop).
   *
   */
#define FT_GX_VALIDATE_H  <freetype/ftgxval.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_PFR_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which accesses PFR-specific data.
   *
   */
#define FT_PFR_H  <freetype/ftpfr.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_STROKER_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which provides functions to stroke outline paths.
   */
#define FT_STROKER_H  <freetype/ftstroke.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_SYNTHESIS_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which performs artificial obliquing and emboldening.
   */
#define FT_SYNTHESIS_H  <freetype/ftsynth.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_XFREE86_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which provides functions specific to the XFree86 and
   *   X.Org X11 servers.
   */
#define FT_XFREE86_H  <freetype/ftxf86.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_TRIGONOMETRY_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which performs trigonometric computations (e.g.,
   *   cosines and arc tangents).
   */
#define FT_TRIGONOMETRY_H  <freetype/fttrigon.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_LCD_FILTER_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which performs color filtering for subpixel rendering.
   */
#define FT_LCD_FILTER_H  <freetype/ftlcdfil.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_UNPATENTED_HINTING_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which performs color filtering for subpixel rendering.
   */
#define FT_UNPATENTED_HINTING_H  <freetype/ttunpat.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_INCREMENTAL_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which performs color filtering for subpixel rendering.
   */
#define FT_INCREMENTAL_H  <freetype/ftincrem.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_GASP_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which returns entries from the TrueType GASP table.
   */
#define FT_GASP_H  <freetype/ftgasp.h>

  /*************************************************************************
   *
   * @macro:
   *   FT_ADVANCES_H
   *
   * @description:
   *   A macro used in #include statements to name the file containing the
   *   FreeType~2 API which returns individual and ranged glyph advances.
   */
#define FT_ADVANCES_H  <freetype/ftadvanc.h>

  /* */

#define FT_ERROR_DEFINITIONS_H  <freetype/fterrdef.h>

  /* The internals of the cache sub-system are no longer exposed.  We */
  /* default to FT_CACHE_H at the moment just in case, but we know of */
  /* no rogue client that uses them.                                  */
  /*                                                                  */
#define FT_CACHE_MANAGER_H           <freetype/ftcache.h>
#define FT_CACHE_INTERNAL_MRU_H      <freetype/ftcache.h>
#define FT_CACHE_INTERNAL_MANAGER_H  <freetype/ftcache.h>
#define FT_CACHE_INTERNAL_CACHE_H    <freetype/ftcache.h>
#define FT_CACHE_INTERNAL_GLYPH_H    <freetype/ftcache.h>
#define FT_CACHE_INTERNAL_IMAGE_H    <freetype/ftcache.h>
#define FT_CACHE_INTERNAL_SBITS_H    <freetype/ftcache.h>

#define FT_INCREMENTAL_H          <freetype/ftincrem.h>

#define FT_TRUETYPE_UNPATENTED_H  <freetype/ttunpat.h>

  /*
   * Include internal headers definitions from <freetype/internal/...>
   * only when building the library.
   */
#ifdef FT2_BUILD_LIBRARY
#define  FT_INTERNAL_INTERNAL_H  <freetype/internal/internal.h>

/*** Start of inlined file: internal.h ***/
/*                                                                         */
/*  internal.h                                                             */
/*                                                                         */
/*    Internal header files (specification only).                          */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file is automatically included by `ft2build.h'.                  */
  /* Do not include it manually!                                           */
  /*                                                                       */
  /*************************************************************************/

#define FT_INTERNAL_OBJECTS_H             <freetype/internal/ftobjs.h>
#define FT_INTERNAL_PIC_H                 <freetype/internal/ftpic.h>
#define FT_INTERNAL_STREAM_H              <freetype/internal/ftstream.h>
#define FT_INTERNAL_MEMORY_H              <freetype/internal/ftmemory.h>
#define FT_INTERNAL_DEBUG_H               <freetype/internal/ftdebug.h>
#define FT_INTERNAL_CALC_H                <freetype/internal/ftcalc.h>
#define FT_INTERNAL_DRIVER_H              <freetype/internal/ftdriver.h>
#define FT_INTERNAL_TRACE_H               <freetype/internal/fttrace.h>
#define FT_INTERNAL_GLYPH_LOADER_H        <freetype/internal/ftgloadr.h>
#define FT_INTERNAL_SFNT_H                <freetype/internal/sfnt.h>
#define FT_INTERNAL_SERVICE_H             <freetype/internal/ftserv.h>
#define FT_INTERNAL_RFORK_H               <freetype/internal/ftrfork.h>
#define FT_INTERNAL_VALIDATE_H            <freetype/internal/ftvalid.h>

#define FT_INTERNAL_TRUETYPE_TYPES_H      <freetype/internal/tttypes.h>
#define FT_INTERNAL_TYPE1_TYPES_H         <freetype/internal/t1types.h>

#define FT_INTERNAL_POSTSCRIPT_AUX_H      <freetype/internal/psaux.h>
#define FT_INTERNAL_POSTSCRIPT_HINTS_H    <freetype/internal/pshints.h>
#define FT_INTERNAL_POSTSCRIPT_GLOBALS_H  <freetype/internal/psglobal.h>

#define FT_INTERNAL_AUTOHINT_H            <freetype/internal/autohint.h>

/* END */

/*** End of inlined file: internal.h ***/


#endif /* FT2_BUILD_LIBRARY */

#endif /* __FT2_BUILD_H__ */

/* END */

/*** End of inlined file: ftheader.h ***/

#endif /* __FT2_BUILD_GENERIC_H__ */

/* END */

/*** End of inlined file: ft2build.h ***/


/*** Start of inlined file: freetype.h ***/
/*                                                                         */
/*  freetype.h                                                             */
/*                                                                         */
/*    FreeType high-level API and common types (specification only).       */
/*                                                                         */
/*  Copyright 1996-2012 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FREETYPE_H__
#define __FREETYPE_H__

#ifndef FT_FREETYPE_H
#error "`ft2build.h' hasn't been included yet!"
#error "Please always use macros to include FreeType header files."
#error "Example:"
#error "  #include <ft2build.h>"
#error "  #include FT_FREETYPE_H"
#endif


/*** Start of inlined file: ftconfig.h ***/
/*                                                                         */
/*  ftconfig.h                                                             */
/*                                                                         */
/*    ANSI-specific configuration file (specification only).               */
/*                                                                         */
/*  Copyright 1996-2004, 2006-2008, 2010-2011 by                           */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This header file contains a number of macro definitions that are used */
  /* by the rest of the engine.  Most of the macros here are automatically */
  /* determined at compile time, and you should not need to change it to   */
  /* port FreeType, except to compile the library with a non-ANSI          */
  /* compiler.                                                             */
  /*                                                                       */
  /* Note however that if some specific modifications are needed, we       */
  /* advise you to place a modified copy in your build directory.          */
  /*                                                                       */
  /* The build directory is usually `freetype/builds/<system>', and        */
  /* contains system-specific files that are always included first when    */
  /* building the library.                                                 */
  /*                                                                       */
  /* This ANSI version should stay in `include/freetype/config'.           */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FTCONFIG_H__
#define __FTCONFIG_H__


/*** Start of inlined file: ftoption.h ***/
/*                                                                         */
/*  ftoption.h                                                             */
/*                                                                         */
/*    User-selectable configuration macros (specification only).           */
/*                                                                         */
/*  Copyright 1996-2012 by                                                 */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FTOPTION_H__
#define __FTOPTION_H__

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /*                 USER-SELECTABLE CONFIGURATION MACROS                  */
  /*                                                                       */
  /* This file contains the default configuration macro definitions for    */
  /* a standard build of the FreeType library.  There are three ways to    */
  /* use this file to build project-specific versions of the library:      */
  /*                                                                       */
  /*  - You can modify this file by hand, but this is not recommended in   */
  /*    cases where you would like to build several versions of the        */
  /*    library from a single source directory.                            */
  /*                                                                       */
  /*  - You can put a copy of this file in your build directory, more      */
  /*    precisely in `$BUILD/freetype/config/ftoption.h', where `$BUILD'   */
  /*    is the name of a directory that is included _before_ the FreeType  */
  /*    include path during compilation.                                   */
  /*                                                                       */
  /*    The default FreeType Makefiles and Jamfiles use the build          */
  /*    directory `builds/<system>' by default, but you can easily change  */
  /*    that for your own projects.                                        */
  /*                                                                       */
  /*  - Copy the file <ft2build.h> to `$BUILD/ft2build.h' and modify it    */
  /*    slightly to pre-define the macro FT_CONFIG_OPTIONS_H used to       */
  /*    locate this file during the build.  For example,                   */
  /*                                                                       */
  /*      #define FT_CONFIG_OPTIONS_H  <myftoptions.h>                     */
  /*      #include <freetype/config/ftheader.h>                            */
  /*                                                                       */
  /*    will use `$BUILD/myftoptions.h' instead of this file for macro     */
  /*    definitions.                                                       */
  /*                                                                       */
  /*    Note also that you can similarly pre-define the macro              */
  /*    FT_CONFIG_MODULES_H used to locate the file listing of the modules */
  /*    that are statically linked to the library at compile time.  By     */
  /*    default, this file is <freetype/config/ftmodule.h>.                */
  /*                                                                       */
  /*  We highly recommend using the third method whenever possible.        */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /**** G E N E R A L   F R E E T Y P E   2   C O N F I G U R A T I O N ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Uncomment the line below if you want to activate sub-pixel rendering  */
  /* (a.k.a. LCD rendering, or ClearType) in this build of the library.    */
  /*                                                                       */
  /* Note that this feature is covered by several Microsoft patents        */
  /* and should not be activated in any default build of the library.      */
  /*                                                                       */
  /* This macro has no impact on the FreeType API, only on its             */
  /* _implementation_.  For example, using FT_RENDER_MODE_LCD when calling */
  /* FT_Render_Glyph still generates a bitmap that is 3 times wider than   */
  /* the original size in case this macro isn't defined; however, each     */
  /* triplet of subpixels has R=G=B.                                       */
  /*                                                                       */
  /* This is done to allow FreeType clients to run unmodified, forcing     */
  /* them to display normal gray-level anti-aliased glyphs.                */
  /*                                                                       */
/* #define FT_CONFIG_OPTION_SUBPIXEL_RENDERING */

  /*************************************************************************/
  /*                                                                       */
  /* Many compilers provide a non-ANSI 64-bit data type that can be used   */
  /* by FreeType to speed up some computations.  However, this will create */
  /* some problems when compiling the library in strict ANSI mode.         */
  /*                                                                       */
  /* For this reason, the use of 64-bit integers is normally disabled when */
  /* the __STDC__ macro is defined.  You can however disable this by       */
  /* defining the macro FT_CONFIG_OPTION_FORCE_INT64 here.                 */
  /*                                                                       */
  /* For most compilers, this will only create compilation warnings when   */
  /* building the library.                                                 */
  /*                                                                       */
  /* ObNote: The compiler-specific 64-bit integers are detected in the     */
  /*         file `ftconfig.h' either statically or through the            */
  /*         `configure' script on supported platforms.                    */
  /*                                                                       */
#undef FT_CONFIG_OPTION_FORCE_INT64

  /*************************************************************************/
  /*                                                                       */
  /* If this macro is defined, do not try to use an assembler version of   */
  /* performance-critical functions (e.g. FT_MulFix).  You should only do  */
  /* that to verify that the assembler function works properly, or to      */
  /* execute benchmark tests of the various implementations.               */
/* #define FT_CONFIG_OPTION_NO_ASSEMBLER */

  /*************************************************************************/
  /*                                                                       */
  /* If this macro is defined, try to use an inlined assembler version of  */
  /* the `FT_MulFix' function, which is a `hotspot' when loading and       */
  /* hinting glyphs, and which should be executed as fast as possible.     */
  /*                                                                       */
  /* Note that if your compiler or CPU is not supported, this will default */
  /* to the standard and portable implementation found in `ftcalc.c'.      */
  /*                                                                       */
#define FT_CONFIG_OPTION_INLINE_MULFIX

  /*************************************************************************/
  /*                                                                       */
  /* LZW-compressed file support.                                          */
  /*                                                                       */
  /*   FreeType now handles font files that have been compressed with the  */
  /*   `compress' program.  This is mostly used to parse many of the PCF   */
  /*   files that come with various X11 distributions.  The implementation */
  /*   uses NetBSD's `zopen' to partially uncompress the file on the fly   */
  /*   (see src/lzw/ftgzip.c).                                             */
  /*                                                                       */
  /*   Define this macro if you want to enable this `feature'.             */
  /*                                                                       */
#define FT_CONFIG_OPTION_USE_LZW

  /*************************************************************************/
  /*                                                                       */
  /* Gzip-compressed file support.                                         */
  /*                                                                       */
  /*   FreeType now handles font files that have been compressed with the  */
  /*   `gzip' program.  This is mostly used to parse many of the PCF files */
  /*   that come with XFree86.  The implementation uses `zlib' to          */
  /*   partially uncompress the file on the fly (see src/gzip/ftgzip.c).   */
  /*                                                                       */
  /*   Define this macro if you want to enable this `feature'.  See also   */
  /*   the macro FT_CONFIG_OPTION_SYSTEM_ZLIB below.                       */
  /*                                                                       */
#define FT_CONFIG_OPTION_USE_ZLIB

  /*************************************************************************/
  /*                                                                       */
  /* ZLib library selection                                                */
  /*                                                                       */
  /*   This macro is only used when FT_CONFIG_OPTION_USE_ZLIB is defined.  */
  /*   It allows FreeType's `ftgzip' component to link to the system's     */
  /*   installation of the ZLib library.  This is useful on systems like   */
  /*   Unix or VMS where it generally is already available.                */
  /*                                                                       */
  /*   If you let it undefined, the component will use its own copy        */
  /*   of the zlib sources instead.  These have been modified to be        */
  /*   included directly within the component and *not* export external    */
  /*   function names.  This allows you to link any program with FreeType  */
  /*   _and_ ZLib without linking conflicts.                               */
  /*                                                                       */
  /*   Do not #undef this macro here since the build system might define   */
  /*   it for certain configurations only.                                 */
  /*                                                                       */
/* #define FT_CONFIG_OPTION_SYSTEM_ZLIB */

  /*************************************************************************/
  /*                                                                       */
  /* Bzip2-compressed file support.                                        */
  /*                                                                       */
  /*   FreeType now handles font files that have been compressed with the  */
  /*   `bzip2' program.  This is mostly used to parse many of the PCF      */
  /*   files that come with XFree86.  The implementation uses `libbz2' to  */
  /*   partially uncompress the file on the fly (see src/bzip2/ftbzip2.c). */
  /*   Contrary to gzip, bzip2 currently is not included and need to use   */
  /*   the system available bzip2 implementation.                          */
  /*                                                                       */
  /*   Define this macro if you want to enable this `feature'.             */
  /*                                                                       */
/* #define FT_CONFIG_OPTION_USE_BZIP2 */

  /*************************************************************************/
  /*                                                                       */
  /* Define to disable the use of file stream functions and types, FILE,   */
  /* fopen() etc.  Enables the use of smaller system libraries on embedded */
  /* systems that have multiple system libraries, some with or without     */
  /* file stream support, in the cases where file stream support is not    */
  /* necessary such as memory loading of font files.                       */
  /*                                                                       */
/* #define FT_CONFIG_OPTION_DISABLE_STREAM_SUPPORT */

  /*************************************************************************/
  /*                                                                       */
  /* DLL export compilation                                                */
  /*                                                                       */
  /*   When compiling FreeType as a DLL, some systems/compilers need a     */
  /*   special keyword in front OR after the return type of function       */
  /*   declarations.                                                       */
  /*                                                                       */
  /*   Two macros are used within the FreeType source code to define       */
  /*   exported library functions: FT_EXPORT and FT_EXPORT_DEF.            */
  /*                                                                       */
  /*     FT_EXPORT( return_type )                                          */
  /*                                                                       */
  /*       is used in a function declaration, as in                        */
  /*                                                                       */
  /*         FT_EXPORT( FT_Error )                                         */
  /*         FT_Init_FreeType( FT_Library*  alibrary );                    */
  /*                                                                       */
  /*                                                                       */
  /*     FT_EXPORT_DEF( return_type )                                      */
  /*                                                                       */
  /*       is used in a function definition, as in                         */
  /*                                                                       */
  /*         FT_EXPORT_DEF( FT_Error )                                     */
  /*         FT_Init_FreeType( FT_Library*  alibrary )                     */
  /*         {                                                             */
  /*           ... some code ...                                           */
  /*           return FT_Err_Ok;                                           */
  /*         }                                                             */
  /*                                                                       */
  /*   You can provide your own implementation of FT_EXPORT and            */
  /*   FT_EXPORT_DEF here if you want.  If you leave them undefined, they  */
  /*   will be later automatically defined as `extern return_type' to      */
  /*   allow normal compilation.                                           */
  /*                                                                       */
  /*   Do not #undef these macros here since the build system might define */
  /*   them for certain configurations only.                               */
  /*                                                                       */
/* #define FT_EXPORT(x)      extern x */
/* #define FT_EXPORT_DEF(x)  x */

  /*************************************************************************/
  /*                                                                       */
  /* Glyph Postscript Names handling                                       */
  /*                                                                       */
  /*   By default, FreeType 2 is compiled with the `psnames' module.  This */
  /*   module is in charge of converting a glyph name string into a        */
  /*   Unicode value, or return a Macintosh standard glyph name for the    */
  /*   use with the TrueType `post' table.                                 */
  /*                                                                       */
  /*   Undefine this macro if you do not want `psnames' compiled in your   */
  /*   build of FreeType.  This has the following effects:                 */
  /*                                                                       */
  /*   - The TrueType driver will provide its own set of glyph names,      */
  /*     if you build it to support postscript names in the TrueType       */
  /*     `post' table.                                                     */
  /*                                                                       */
  /*   - The Type 1 driver will not be able to synthesize a Unicode        */
  /*     charmap out of the glyphs found in the fonts.                     */
  /*                                                                       */
  /*   You would normally undefine this configuration macro when building  */
  /*   a version of FreeType that doesn't contain a Type 1 or CFF driver.  */
  /*                                                                       */
#define FT_CONFIG_OPTION_POSTSCRIPT_NAMES

  /*************************************************************************/
  /*                                                                       */
  /* Postscript Names to Unicode Values support                            */
  /*                                                                       */
  /*   By default, FreeType 2 is built with the `PSNames' module compiled  */
  /*   in.  Among other things, the module is used to convert a glyph name */
  /*   into a Unicode value.  This is especially useful in order to        */
  /*   synthesize on the fly a Unicode charmap from the CFF/Type 1 driver  */
  /*   through a big table named the `Adobe Glyph List' (AGL).             */
  /*                                                                       */
  /*   Undefine this macro if you do not want the Adobe Glyph List         */
  /*   compiled in your `PSNames' module.  The Type 1 driver will not be   */
  /*   able to synthesize a Unicode charmap out of the glyphs found in the */
  /*   fonts.                                                              */
  /*                                                                       */
#define FT_CONFIG_OPTION_ADOBE_GLYPH_LIST

  /*************************************************************************/
  /*                                                                       */
  /* Support for Mac fonts                                                 */
  /*                                                                       */
  /*   Define this macro if you want support for outline fonts in Mac      */
  /*   format (mac dfont, mac resource, macbinary containing a mac         */
  /*   resource) on non-Mac platforms.                                     */
  /*                                                                       */
  /*   Note that the `FOND' resource isn't checked.                        */
  /*                                                                       */
#define FT_CONFIG_OPTION_MAC_FONTS

  /*************************************************************************/
  /*                                                                       */
  /* Guessing methods to access embedded resource forks                    */
  /*                                                                       */
  /*   Enable extra Mac fonts support on non-Mac platforms (e.g.           */
  /*   GNU/Linux).                                                         */
  /*                                                                       */
  /*   Resource forks which include fonts data are stored sometimes in     */
  /*   locations which users or developers don't expected.  In some cases, */
  /*   resource forks start with some offset from the head of a file.  In  */
  /*   other cases, the actual resource fork is stored in file different   */
  /*   from what the user specifies.  If this option is activated,         */
  /*   FreeType tries to guess whether such offsets or different file      */
  /*   names must be used.                                                 */
  /*                                                                       */
  /*   Note that normal, direct access of resource forks is controlled via */
  /*   the FT_CONFIG_OPTION_MAC_FONTS option.                              */
  /*                                                                       */
#ifdef FT_CONFIG_OPTION_MAC_FONTS
#define FT_CONFIG_OPTION_GUESSING_EMBEDDED_RFORK
#endif

  /*************************************************************************/
  /*                                                                       */
  /* Allow the use of FT_Incremental_Interface to load typefaces that      */
  /* contain no glyph data, but supply it via a callback function.         */
  /* This is required by clients supporting document formats which         */
  /* supply font data incrementally as the document is parsed, such        */
  /* as the Ghostscript interpreter for the PostScript language.           */
  /*                                                                       */
#define FT_CONFIG_OPTION_INCREMENTAL

  /*************************************************************************/
  /*                                                                       */
  /* The size in bytes of the render pool used by the scan-line converter  */
  /* to do all of its work.                                                */
  /*                                                                       */
  /* This must be greater than 4KByte if you use FreeType to rasterize     */
  /* glyphs; otherwise, you may set it to zero to avoid unnecessary        */
  /* allocation of the render pool.                                        */
  /*                                                                       */
#define FT_RENDER_POOL_SIZE  16384L

  /*************************************************************************/
  /*                                                                       */
  /* FT_MAX_MODULES                                                        */
  /*                                                                       */
  /*   The maximum number of modules that can be registered in a single    */
  /*   FreeType library object.  32 is the default.                        */
  /*                                                                       */
#define FT_MAX_MODULES  32

  /*************************************************************************/
  /*                                                                       */
  /* Debug level                                                           */
  /*                                                                       */
  /*   FreeType can be compiled in debug or trace mode.  In debug mode,    */
  /*   errors are reported through the `ftdebug' component.  In trace      */
  /*   mode, additional messages are sent to the standard output during    */
  /*   execution.                                                          */
  /*                                                                       */
  /*   Define FT_DEBUG_LEVEL_ERROR to build the library in debug mode.     */
  /*   Define FT_DEBUG_LEVEL_TRACE to build it in trace mode.              */
  /*                                                                       */
  /*   Don't define any of these macros to compile in `release' mode!      */
  /*                                                                       */
  /*   Do not #undef these macros here since the build system might define */
  /*   them for certain configurations only.                               */
  /*                                                                       */
/* #define FT_DEBUG_LEVEL_ERROR */
/* #define FT_DEBUG_LEVEL_TRACE */

  /*************************************************************************/
  /*                                                                       */
  /* Autofitter debugging                                                  */
  /*                                                                       */
  /*   If FT_DEBUG_AUTOFIT is defined, FreeType provides some means to     */
  /*   control the autofitter behaviour for debugging purposes with global */
  /*   boolean variables (consequently, you should *never* enable this     */
  /*   while compiling in `release' mode):                                 */
  /*                                                                       */
  /*     _af_debug_disable_horz_hints                                      */
  /*     _af_debug_disable_vert_hints                                      */
  /*     _af_debug_disable_blue_hints                                      */
  /*                                                                       */
  /*   Additionally, the following functions provide dumps of various      */
  /*   internal autofit structures to stdout (using `printf'):             */
  /*                                                                       */
  /*     af_glyph_hints_dump_points                                        */
  /*     af_glyph_hints_dump_segments                                      */
  /*     af_glyph_hints_dump_edges                                         */
  /*                                                                       */
  /*   As an argument, they use another global variable:                   */
  /*                                                                       */
  /*     _af_debug_hints                                                   */
  /*                                                                       */
  /*   Please have a look at the `ftgrid' demo program to see how those    */
  /*   variables and macros should be used.                                */
  /*                                                                       */
  /*   Do not #undef these macros here since the build system might define */
  /*   them for certain configurations only.                               */
  /*                                                                       */
/* #define FT_DEBUG_AUTOFIT */

  /*************************************************************************/
  /*                                                                       */
  /* Memory Debugging                                                      */
  /*                                                                       */
  /*   FreeType now comes with an integrated memory debugger that is       */
  /*   capable of detecting simple errors like memory leaks or double      */
  /*   deletes.  To compile it within your build of the library, you       */
  /*   should define FT_DEBUG_MEMORY here.                                 */
  /*                                                                       */
  /*   Note that the memory debugger is only activated at runtime when     */
  /*   when the _environment_ variable `FT2_DEBUG_MEMORY' is defined also! */
  /*                                                                       */
  /*   Do not #undef this macro here since the build system might define   */
  /*   it for certain configurations only.                                 */
  /*                                                                       */
/* #define FT_DEBUG_MEMORY */

  /*************************************************************************/
  /*                                                                       */
  /* Module errors                                                         */
  /*                                                                       */
  /*   If this macro is set (which is _not_ the default), the higher byte  */
  /*   of an error code gives the module in which the error has occurred,  */
  /*   while the lower byte is the real error code.                        */
  /*                                                                       */
  /*   Setting this macro makes sense for debugging purposes only, since   */
  /*   it would break source compatibility of certain programs that use    */
  /*   FreeType 2.                                                         */
  /*                                                                       */
  /*   More details can be found in the files ftmoderr.h and fterrors.h.   */
  /*                                                                       */
#undef FT_CONFIG_OPTION_USE_MODULE_ERRORS

  /*************************************************************************/
  /*                                                                       */
  /* Position Independent Code                                             */
  /*                                                                       */
  /*   If this macro is set (which is _not_ the default), FreeType2 will   */
  /*   avoid creating constants that require address fixups.  Instead the  */
  /*   constants will be moved into a struct and additional intialization  */
  /*   code will be used.                                                  */
  /*                                                                       */
  /*   Setting this macro is needed for systems that prohibit address      */
  /*   fixups, such as BREW.                                               */
  /*                                                                       */
/* #define FT_CONFIG_OPTION_PIC */

  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****        S F N T   D R I V E R    C O N F I G U R A T I O N       ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_EMBEDDED_BITMAPS if you want to support       */
  /* embedded bitmaps in all formats using the SFNT module (namely         */
  /* TrueType & OpenType).                                                 */
  /*                                                                       */
#define TT_CONFIG_OPTION_EMBEDDED_BITMAPS

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_POSTSCRIPT_NAMES if you want to be able to    */
  /* load and enumerate the glyph Postscript names in a TrueType or        */
  /* OpenType file.                                                        */
  /*                                                                       */
  /* Note that when you do not compile the `PSNames' module by undefining  */
  /* the above FT_CONFIG_OPTION_POSTSCRIPT_NAMES, the `sfnt' module will   */
  /* contain additional code used to read the PS Names table from a font.  */
  /*                                                                       */
  /* (By default, the module uses `PSNames' to extract glyph names.)       */
  /*                                                                       */
#define TT_CONFIG_OPTION_POSTSCRIPT_NAMES

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_SFNT_NAMES if your applications need to       */
  /* access the internal name table in a SFNT-based format like TrueType   */
  /* or OpenType.  The name table contains various strings used to         */
  /* describe the font, like family name, copyright, version, etc.  It     */
  /* does not contain any glyph name though.                               */
  /*                                                                       */
  /* Accessing SFNT names is done through the functions declared in        */
  /* `freetype/ftsnames.h'.                                                */
  /*                                                                       */
#define TT_CONFIG_OPTION_SFNT_NAMES

  /*************************************************************************/
  /*                                                                       */
  /* TrueType CMap support                                                 */
  /*                                                                       */
  /*   Here you can fine-tune which TrueType CMap table format shall be    */
  /*   supported.                                                          */
#define TT_CONFIG_CMAP_FORMAT_0
#define TT_CONFIG_CMAP_FORMAT_2
#define TT_CONFIG_CMAP_FORMAT_4
#define TT_CONFIG_CMAP_FORMAT_6
#define TT_CONFIG_CMAP_FORMAT_8
#define TT_CONFIG_CMAP_FORMAT_10
#define TT_CONFIG_CMAP_FORMAT_12
#define TT_CONFIG_CMAP_FORMAT_13
#define TT_CONFIG_CMAP_FORMAT_14

  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****    T R U E T Y P E   D R I V E R    C O N F I G U R A T I O N   ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_BYTECODE_INTERPRETER if you want to compile   */
  /* a bytecode interpreter in the TrueType driver.                        */
  /*                                                                       */
  /* By undefining this, you will only compile the code necessary to load  */
  /* TrueType glyphs without hinting.                                      */
  /*                                                                       */
  /*   Do not #undef this macro here, since the build system might         */
  /*   define it for certain configurations only.                          */
  /*                                                                       */
#define TT_CONFIG_OPTION_BYTECODE_INTERPRETER

  /*************************************************************************/
  /*                                                                       */
  /* If you define TT_CONFIG_OPTION_UNPATENTED_HINTING, a special version  */
  /* of the TrueType bytecode interpreter is used that doesn't implement   */
  /* any of the patented opcodes and algorithms.  The patents related to   */
  /* TrueType hinting have expired worldwide since May 2010; this option   */
  /* is now deprecated.                                                    */
  /*                                                                       */
  /* Note that the TT_CONFIG_OPTION_UNPATENTED_HINTING macro is *ignored*  */
  /* if you define TT_CONFIG_OPTION_BYTECODE_INTERPRETER; in other words,  */
  /* either define TT_CONFIG_OPTION_BYTECODE_INTERPRETER or                */
  /* TT_CONFIG_OPTION_UNPATENTED_HINTING but not both at the same time.    */
  /*                                                                       */
  /* This macro is only useful for a small number of font files (mostly    */
  /* for Asian scripts) that require bytecode interpretation to properly   */
  /* load glyphs.  For all other fonts, this produces unpleasant results,  */
  /* thus the unpatented interpreter is never used to load glyphs from     */
  /* TrueType fonts unless one of the following two options is used.       */
  /*                                                                       */
  /*   - The unpatented interpreter is explicitly activated by the user    */
  /*     through the FT_PARAM_TAG_UNPATENTED_HINTING parameter tag         */
  /*     when opening the FT_Face.                                         */
  /*                                                                       */
  /*   - FreeType detects that the FT_Face corresponds to one of the       */
  /*     `trick' fonts (e.g., `Mingliu') it knows about.  The font engine  */
  /*     contains a hard-coded list of font names and other matching       */
  /*     parameters (see function `tt_face_init' in file                   */
  /*     `src/truetype/ttobjs.c').                                         */
  /*                                                                       */
  /* Here a sample code snippet for using FT_PARAM_TAG_UNPATENTED_HINTING. */
  /*                                                                       */
  /*   {                                                                   */
  /*     FT_Parameter  parameter;                                          */
  /*     FT_Open_Args  open_args;                                          */
  /*                                                                       */
  /*                                                                       */
  /*     parameter.tag = FT_PARAM_TAG_UNPATENTED_HINTING;                  */
  /*                                                                       */
  /*     open_args.flags      = FT_OPEN_PATHNAME | FT_OPEN_PARAMS;         */
  /*     open_args.pathname   = my_font_pathname;                          */
  /*     open_args.num_params = 1;                                         */
  /*     open_args.params     = &parameter;                                */
  /*                                                                       */
  /*     error = FT_Open_Face( library, &open_args, index, &face );        */
  /*     ...                                                               */
  /*   }                                                                   */
  /*                                                                       */
/* #define TT_CONFIG_OPTION_UNPATENTED_HINTING */

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_INTERPRETER_SWITCH to compile the TrueType    */
  /* bytecode interpreter with a huge switch statement, rather than a call */
  /* table.  This results in smaller and faster code for a number of       */
  /* architectures.                                                        */
  /*                                                                       */
  /* Note however that on some compiler/processor combinations, undefining */
  /* this macro will generate faster, though larger, code.                 */
  /*                                                                       */
#define TT_CONFIG_OPTION_INTERPRETER_SWITCH

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_COMPONENT_OFFSET_SCALED to compile the        */
  /* TrueType glyph loader to use Apple's definition of how to handle      */
  /* component offsets in composite glyphs.                                */
  /*                                                                       */
  /* Apple and MS disagree on the default behavior of component offsets    */
  /* in composites.  Apple says that they should be scaled by the scaling  */
  /* factors in the transformation matrix (roughly, it's more complex)     */
  /* while MS says they should not.  OpenType defines two bits in the      */
  /* composite flags array which can be used to disambiguate, but old      */
  /* fonts will not have them.                                             */
  /*                                                                       */
  /*   http://www.microsoft.com/typography/otspec/glyf.htm                 */
  /*   http://fonts.apple.com/TTRefMan/RM06/Chap6glyf.html                 */
  /*                                                                       */
#undef TT_CONFIG_OPTION_COMPONENT_OFFSET_SCALED

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_GX_VAR_SUPPORT if you want to include         */
  /* support for Apple's distortable font technology (fvar, gvar, cvar,    */
  /* and avar tables).  This has many similarities to Type 1 Multiple      */
  /* Masters support.                                                      */
  /*                                                                       */
#define TT_CONFIG_OPTION_GX_VAR_SUPPORT

  /*************************************************************************/
  /*                                                                       */
  /* Define TT_CONFIG_OPTION_BDF if you want to include support for        */
  /* an embedded `BDF ' table within SFNT-based bitmap formats.            */
  /*                                                                       */
#define TT_CONFIG_OPTION_BDF

  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****      T Y P E 1   D R I V E R    C O N F I G U R A T I O N       ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* T1_MAX_DICT_DEPTH is the maximum depth of nest dictionaries and       */
  /* arrays in the Type 1 stream (see t1load.c).  A minimum of 4 is        */
  /* required.                                                             */
  /*                                                                       */
#define T1_MAX_DICT_DEPTH  5

  /*************************************************************************/
  /*                                                                       */
  /* T1_MAX_SUBRS_CALLS details the maximum number of nested sub-routine   */
  /* calls during glyph loading.                                           */
  /*                                                                       */
#define T1_MAX_SUBRS_CALLS  16

  /*************************************************************************/
  /*                                                                       */
  /* T1_MAX_CHARSTRING_OPERANDS is the charstring stack's capacity.  A     */
  /* minimum of 16 is required.                                            */
  /*                                                                       */
  /* The Chinese font MingTiEG-Medium (CNS 11643 character set) needs 256. */
  /*                                                                       */
#define T1_MAX_CHARSTRINGS_OPERANDS  256

  /*************************************************************************/
  /*                                                                       */
  /* Define this configuration macro if you want to prevent the            */
  /* compilation of `t1afm', which is in charge of reading Type 1 AFM      */
  /* files into an existing face.  Note that if set, the T1 driver will be */
  /* unable to produce kerning distances.                                  */
  /*                                                                       */
#undef T1_CONFIG_OPTION_NO_AFM

  /*************************************************************************/
  /*                                                                       */
  /* Define this configuration macro if you want to prevent the            */
  /* compilation of the Multiple Masters font support in the Type 1        */
  /* driver.                                                               */
  /*                                                                       */
#undef T1_CONFIG_OPTION_NO_MM_SUPPORT

  /*************************************************************************/
  /*************************************************************************/
  /****                                                                 ****/
  /****    A U T O F I T   M O D U L E    C O N F I G U R A T I O N     ****/
  /****                                                                 ****/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Compile autofit module with CJK (Chinese, Japanese, Korean) script    */
  /* support.                                                              */
  /*                                                                       */
#define AF_CONFIG_OPTION_CJK

  /*************************************************************************/
  /*                                                                       */
  /* Compile autofit module with Indic script support.                     */
  /*                                                                       */
#define AF_CONFIG_OPTION_INDIC

  /*************************************************************************/
  /*                                                                       */
  /* Compile autofit module with warp hinting.  The idea of the warping    */
  /* code is to slightly scale and shift a glyph within a single dimension */
  /* so that as much of its segments are aligned (more or less) on the     */
  /* grid.  To find out the optimal scaling and shifting value, various    */
  /* parameter combinations are tried and scored.                          */
  /*                                                                       */
  /* This experimental option is only active if the render mode is         */
  /* FT_RENDER_MODE_LIGHT.                                                 */
  /*                                                                       */
/* #define AF_CONFIG_OPTION_USE_WARPER */

  /* */

  /*
   * Define this variable if you want to keep the layout of internal
   * structures that was used prior to FreeType 2.2.  This also compiles in
   * a few obsolete functions to avoid linking problems on typical Unix
   * distributions.
   *
   * For embedded systems or building a new distribution from scratch, it
   * is recommended to disable the macro since it reduces the library's code
   * size and activates a few memory-saving optimizations as well.
   */
#define FT_CONFIG_OPTION_OLD_INTERNALS

  /*
   *  To detect legacy cache-lookup call from a rogue client (<= 2.1.7),
   *  we restrict the number of charmaps in a font.  The current API of
   *  FTC_CMapCache_Lookup() takes cmap_index & charcode, but old API
   *  takes charcode only.  To determine the passed value is for cmap_index
   *  or charcode, the possible cmap_index is restricted not to exceed
   *  the minimum possible charcode by a rogue client.  It is also very
   *  unlikely that a rogue client is interested in Unicode values 0 to 15.
   *
   *  NOTE: The original threshold was 4 deduced from popular number of
   *        cmap subtables in UCS-4 TrueType fonts, but now it is not
   *        irregular for OpenType fonts to have more than 4 subtables,
   *        because variation selector subtables are available for Apple
   *        and Microsoft platforms.
   */

#ifdef FT_CONFIG_OPTION_OLD_INTERNALS
#define FT_MAX_CHARMAP_CACHEABLE 15
#endif

  /*
   * This macro is defined if either unpatented or native TrueType
   * hinting is requested by the definitions above.
   */
#ifdef TT_CONFIG_OPTION_BYTECODE_INTERPRETER
#define  TT_USE_BYTECODE_INTERPRETER
#undef   TT_CONFIG_OPTION_UNPATENTED_HINTING
#elif defined TT_CONFIG_OPTION_UNPATENTED_HINTING
#define  TT_USE_BYTECODE_INTERPRETER
#endif

FT_END_HEADER

#endif /* __FTOPTION_H__ */

/* END */

/*** End of inlined file: ftoption.h ***/


/*** Start of inlined file: ftstdlib.h ***/
/*                                                                         */
/*  ftstdlib.h                                                             */
/*                                                                         */
/*    ANSI-specific library and header configuration file (specification   */
/*    only).                                                               */
/*                                                                         */
/*  Copyright 2002-2007, 2009, 2011 by                                     */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file is used to group all #includes to the ANSI C library that   */
  /* FreeType normally requires.  It also defines macros to rename the     */
  /* standard functions within the FreeType source code.                   */
  /*                                                                       */
  /* Load a file which defines __FTSTDLIB_H__ before this one to override  */
  /* it.                                                                   */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FTSTDLIB_H__
#define __FTSTDLIB_H__

#include <stddef.h>

#define ft_ptrdiff_t  ptrdiff_t

  /**********************************************************************/
  /*                                                                    */
  /*                           integer limits                           */
  /*                                                                    */
  /* UINT_MAX and ULONG_MAX are used to automatically compute the size  */
  /* of `int' and `long' in bytes at compile-time.  So far, this works  */
  /* for all platforms the library has been tested on.                  */
  /*                                                                    */
  /* Note that on the extremely rare platforms that do not provide      */
  /* integer types that are _exactly_ 16 and 32 bits wide (e.g. some    */
  /* old Crays where `int' is 36 bits), we do not make any guarantee    */
  /* about the correct behaviour of FT2 with all fonts.                 */
  /*                                                                    */
  /* In these case, `ftconfig.h' will refuse to compile anyway with a   */
  /* message like `couldn't find 32-bit type' or something similar.     */
  /*                                                                    */
  /**********************************************************************/

#include <limits.h>

#define FT_CHAR_BIT    CHAR_BIT
#define FT_USHORT_MAX  USHRT_MAX
#define FT_INT_MAX     INT_MAX
#define FT_INT_MIN     INT_MIN
#define FT_UINT_MAX    UINT_MAX
#define FT_ULONG_MAX   ULONG_MAX

  /**********************************************************************/
  /*                                                                    */
  /*                 character and string processing                    */
  /*                                                                    */
  /**********************************************************************/

#include <string.h>

#define ft_memchr   memchr
#define ft_memcmp   memcmp
#define ft_memcpy   memcpy
#define ft_memmove  memmove
#define ft_memset   memset
#define ft_strcat   strcat
#define ft_strcmp   strcmp
#define ft_strcpy   strcpy
#define ft_strlen   strlen
#define ft_strncmp  strncmp
#define ft_strncpy  strncpy
#define ft_strrchr  strrchr
#define ft_strstr   strstr

  /**********************************************************************/
  /*                                                                    */
  /*                           file handling                            */
  /*                                                                    */
  /**********************************************************************/

#include <stdio.h>

#define FT_FILE     FILE
#define ft_fclose   fclose
#define ft_fopen    fopen
#define ft_fread    fread
#define ft_fseek    fseek
#define ft_ftell    ftell
#define ft_sprintf  sprintf

  /**********************************************************************/
  /*                                                                    */
  /*                             sorting                                */
  /*                                                                    */
  /**********************************************************************/

#include <stdlib.h>

#define ft_qsort  qsort

  /**********************************************************************/
  /*                                                                    */
  /*                        memory allocation                           */
  /*                                                                    */
  /**********************************************************************/

#define ft_scalloc   calloc
#define ft_sfree     free
#define ft_smalloc   malloc
#define ft_srealloc  realloc

  /**********************************************************************/
  /*                                                                    */
  /*                          miscellaneous                             */
  /*                                                                    */
  /**********************************************************************/

#define ft_atol   atol
#define ft_labs   labs

  /**********************************************************************/
  /*                                                                    */
  /*                         execution control                          */
  /*                                                                    */
  /**********************************************************************/

#include <setjmp.h>

#define ft_jmp_buf     jmp_buf  /* note: this cannot be a typedef since */
								/*       jmp_buf is defined as a macro  */
								/*       on certain platforms           */

#define ft_longjmp     longjmp
#define ft_setjmp( b ) setjmp( *(jmp_buf*) &(b) )    /* same thing here */

  /* the following is only used for debugging purposes, i.e., if */
  /* FT_DEBUG_LEVEL_ERROR or FT_DEBUG_LEVEL_TRACE are defined    */

#include <stdarg.h>

#endif /* __FTSTDLIB_H__ */

/* END */

/*** End of inlined file: ftstdlib.h ***/

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /*               PLATFORM-SPECIFIC CONFIGURATION MACROS                  */
  /*                                                                       */
  /* These macros can be toggled to suit a specific system.  The current   */
  /* ones are defaults used to compile FreeType in an ANSI C environment   */
  /* (16bit compilers are also supported).  Copy this file to your own     */
  /* `freetype/builds/<system>' directory, and edit it to port the engine. */
  /*                                                                       */
  /*************************************************************************/

  /* There are systems (like the Texas Instruments 'C54x) where a `char' */
  /* has 16 bits.  ANSI C says that sizeof(char) is always 1.  Since an  */
  /* `int' has 16 bits also for this system, sizeof(int) gives 1 which   */
  /* is probably unexpected.                                             */
  /*                                                                     */
  /* `CHAR_BIT' (defined in limits.h) gives the number of bits in a      */
  /* `char' type.                                                        */

#ifndef FT_CHAR_BIT
#define FT_CHAR_BIT  CHAR_BIT
#endif

  /* The size of an `int' type.  */
#if                                 FT_UINT_MAX == 0xFFFFUL
#define FT_SIZEOF_INT  (16 / FT_CHAR_BIT)
#elif                               FT_UINT_MAX == 0xFFFFFFFFUL
#define FT_SIZEOF_INT  (32 / FT_CHAR_BIT)
#elif FT_UINT_MAX > 0xFFFFFFFFUL && FT_UINT_MAX == 0xFFFFFFFFFFFFFFFFUL
#define FT_SIZEOF_INT  (64 / FT_CHAR_BIT)
#else
#error "Unsupported size of `int' type!"
#endif

  /* The size of a `long' type.  A five-byte `long' (as used e.g. on the */
  /* DM642) is recognized but avoided.                                   */
#if                                  FT_ULONG_MAX == 0xFFFFFFFFUL
#define FT_SIZEOF_LONG  (32 / FT_CHAR_BIT)
#elif FT_ULONG_MAX > 0xFFFFFFFFUL && FT_ULONG_MAX == 0xFFFFFFFFFFUL
#define FT_SIZEOF_LONG  (32 / FT_CHAR_BIT)
#elif FT_ULONG_MAX > 0xFFFFFFFFUL && FT_ULONG_MAX == 0xFFFFFFFFFFFFFFFFUL
#define FT_SIZEOF_LONG  (64 / FT_CHAR_BIT)
#else
#error "Unsupported size of `long' type!"
#endif

  /* FT_UNUSED is a macro used to indicate that a given parameter is not  */
  /* used -- this is only used to get rid of unpleasant compiler warnings */
#ifndef FT_UNUSED
#define FT_UNUSED( arg )  ( (arg) = (arg) )
#endif

  /*************************************************************************/
  /*                                                                       */
  /*                     AUTOMATIC CONFIGURATION MACROS                    */
  /*                                                                       */
  /* These macros are computed from the ones defined above.  Don't touch   */
  /* their definition, unless you know precisely what you are doing.  No   */
  /* porter should need to mess with them.                                 */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Mac support                                                           */
  /*                                                                       */
  /*   This is the only necessary change, so it is defined here instead    */
  /*   providing a new configuration file.                                 */
  /*                                                                       */
#if defined( __APPLE__ ) || ( defined( __MWERKS__ ) && defined( macintosh ) )
  /* no Carbon frameworks for 64bit 10.4.x */
  /* AvailabilityMacros.h is available since Mac OS X 10.2,        */
  /* so guess the system version by maximum errno before inclusion */
#include <errno.h>
#ifdef ECANCELED /* defined since 10.2 */
#include "AvailabilityMacros.h"
#endif
#if defined( __LP64__ ) && \
	( MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4 )
#undef FT_MACINTOSH
#endif

#elif defined( __SC__ ) || defined( __MRC__ )
  /* Classic MacOS compilers */
#include "ConditionalMacros.h"
#if TARGET_OS_MAC
#define FT_MACINTOSH 1
#endif

#endif

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    basic_types                                                        */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Int16                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for a 16bit signed integer type.                         */
  /*                                                                       */
  typedef signed short  FT_Int16;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UInt16                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for a 16bit unsigned integer type.                       */
  /*                                                                       */
  typedef unsigned short  FT_UInt16;

  /* */

  /* this #if 0 ... #endif clause is for documentation purposes */
#if 0

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Int32                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for a 32bit signed integer type.  The size depends on    */
  /*    the configuration.                                                 */
  /*                                                                       */
  typedef signed XXX  FT_Int32;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UInt32                                                          */
  /*                                                                       */
  /*    A typedef for a 32bit unsigned integer type.  The size depends on  */
  /*    the configuration.                                                 */
  /*                                                                       */
  typedef unsigned XXX  FT_UInt32;

  /* */

#endif

#if FT_SIZEOF_INT == (32 / FT_CHAR_BIT)

  typedef signed int      FT_Int32;
  typedef unsigned int    FT_UInt32;

#elif FT_SIZEOF_LONG == (32 / FT_CHAR_BIT)

  typedef signed long     FT_Int32;
  typedef unsigned long   FT_UInt32;

#else
#error "no 32bit type found -- please check your configuration files"
#endif

  /* look up an integer type that is at least 32 bits */
#if FT_SIZEOF_INT >= (32 / FT_CHAR_BIT)

  typedef int            FT_Fast;
  typedef unsigned int   FT_UFast;

#elif FT_SIZEOF_LONG >= (32 / FT_CHAR_BIT)

  typedef long           FT_Fast;
  typedef unsigned long  FT_UFast;

#endif

  /* determine whether we have a 64-bit int type for platforms without */
  /* Autoconf                                                          */
#if FT_SIZEOF_LONG == (64 / FT_CHAR_BIT)

  /* FT_LONG64 must be defined if a 64-bit type is available */
#define FT_LONG64
#define FT_INT64  long

#elif defined( _MSC_VER ) && _MSC_VER >= 900  /* Visual C++ (and Intel C++) */

  /* this compiler provides the __int64 type */
#define FT_LONG64
#define FT_INT64  __int64

#elif defined( __BORLANDC__ )  /* Borland C++ */

  /* XXXX: We should probably check the value of __BORLANDC__ in order */
  /*       to test the compiler version.                               */

  /* this compiler provides the __int64 type */
#define FT_LONG64
#define FT_INT64  __int64

#elif defined( __WATCOMC__ )   /* Watcom C++ */

  /* Watcom doesn't provide 64-bit data types */

#elif defined( __MWERKS__ )    /* Metrowerks CodeWarrior */

#define FT_LONG64
#define FT_INT64  long long int

#elif defined( __GNUC__ )

  /* GCC provides the `long long' type */
#define FT_LONG64
#define FT_INT64  long long int

#endif /* FT_SIZEOF_LONG == (64 / FT_CHAR_BIT) */

  /*************************************************************************/
  /*                                                                       */
  /* A 64-bit data type will create compilation problems if you compile    */
  /* in strict ANSI mode.  To avoid them, we disable its use if __STDC__   */
  /* is defined.  You can however ignore this rule by defining the         */
  /* FT_CONFIG_OPTION_FORCE_INT64 configuration macro.                     */
  /*                                                                       */
#if defined( FT_LONG64 ) && !defined( FT_CONFIG_OPTION_FORCE_INT64 )

#ifdef __STDC__

  /* undefine the 64-bit macros in strict ANSI compilation mode */
#undef FT_LONG64
#undef FT_INT64

#endif /* __STDC__ */

#endif /* FT_LONG64 && !FT_CONFIG_OPTION_FORCE_INT64 */

#define FT_BEGIN_STMNT  do {
#define FT_END_STMNT    } while ( 0 )
#define FT_DUMMY_STMNT  FT_BEGIN_STMNT FT_END_STMNT

#ifndef  FT_CONFIG_OPTION_NO_ASSEMBLER
  /* Provide assembler fragments for performance-critical functions. */
  /* These must be defined `static __inline__' with GCC.             */

#if defined( __CC_ARM ) || defined( __ARMCC__ )  /* RVCT */
#define FT_MULFIX_ASSEMBLER  FT_MulFix_arm

  /* documentation is in freetype.h */

  static __inline FT_Int32
  FT_MulFix_arm( FT_Int32  a,
				 FT_Int32  b )
  {
	register FT_Int32  t, t2;

	__asm
	{
	  smull t2, t,  b,  a           /* (lo=t2,hi=t) = a*b */
	  mov   a,  t,  asr #31         /* a   = (hi >> 31) */
	  add   a,  a,  #0x8000         /* a  += 0x8000 */
	  adds  t2, t2, a               /* t2 += a */
	  adc   t,  t,  #0              /* t  += carry */
	  mov   a,  t2, lsr #16         /* a   = t2 >> 16 */
	  orr   a,  a,  t,  lsl #16     /* a  |= t << 16 */
	}
	return a;
  }

#endif /* __CC_ARM || __ARMCC__ */

#ifdef __GNUC__

#if defined( __arm__ ) && !defined( __thumb__ )    && \
	!( defined( __CC_ARM ) || defined( __ARMCC__ ) )
#define FT_MULFIX_ASSEMBLER  FT_MulFix_arm

  /* documentation is in freetype.h */

  static __inline__ FT_Int32
  FT_MulFix_arm( FT_Int32  a,
				 FT_Int32  b )
  {
	register FT_Int32  t, t2;

	__asm__ __volatile__ (
	  "smull  %1, %2, %4, %3\n\t"       /* (lo=%1,hi=%2) = a*b */
	  "mov    %0, %2, asr #31\n\t"      /* %0  = (hi >> 31) */
	  "add    %0, %0, #0x8000\n\t"      /* %0 += 0x8000 */
	  "adds   %1, %1, %0\n\t"           /* %1 += %0 */
	  "adc    %2, %2, #0\n\t"           /* %2 += carry */
	  "mov    %0, %1, lsr #16\n\t"      /* %0  = %1 >> 16 */
	  "orr    %0, %0, %2, lsl #16\n\t"  /* %0 |= %2 << 16 */
	  : "=r"(a), "=&r"(t2), "=&r"(t)
	  : "r"(a), "r"(b) );
	return a;
  }

#endif /* __arm__ && !__thumb__ && !( __CC_ARM || __ARMCC__ ) */

#if defined( __i386__ )
#define FT_MULFIX_ASSEMBLER  FT_MulFix_i386

  /* documentation is in freetype.h */

  static __inline__ FT_Int32
  FT_MulFix_i386( FT_Int32  a,
				  FT_Int32  b )
  {
	register FT_Int32  result;

	__asm__ __volatile__ (
	  "imul  %%edx\n"
	  "movl  %%edx, %%ecx\n"
	  "sarl  $31, %%ecx\n"
	  "addl  $0x8000, %%ecx\n"
	  "addl  %%ecx, %%eax\n"
	  "adcl  $0, %%edx\n"
	  "shrl  $16, %%eax\n"
	  "shll  $16, %%edx\n"
	  "addl  %%edx, %%eax\n"
	  : "=a"(result), "=d"(b)
	  : "a"(a), "d"(b)
	  : "%ecx", "cc" );
	return result;
  }

#endif /* i386 */

#endif /* __GNUC__ */

#ifdef _MSC_VER /* Visual C++ */

#ifdef _M_IX86

#define FT_MULFIX_ASSEMBLER  FT_MulFix_i386

  /* documentation is in freetype.h */

  static __inline FT_Int32
  FT_MulFix_i386( FT_Int32  a,
				  FT_Int32  b )
  {
	register FT_Int32  result;

	__asm
	{
	  mov eax, a
	  mov edx, b
	  imul edx
	  mov ecx, edx
	  sar ecx, 31
	  add ecx, 8000h
	  add eax, ecx
	  adc edx, 0
	  shr eax, 16
	  shl edx, 16
	  add eax, edx
	  mov result, eax
	}
	return result;
  }

#endif /* _M_IX86 */

#endif /* _MSC_VER */

#endif /* !FT_CONFIG_OPTION_NO_ASSEMBLER */

#ifdef FT_CONFIG_OPTION_INLINE_MULFIX
#ifdef FT_MULFIX_ASSEMBLER
#define FT_MULFIX_INLINED  FT_MULFIX_ASSEMBLER
#endif
#endif

#ifdef FT_MAKE_OPTION_SINGLE_OBJECT

#define FT_LOCAL( x )      static  x
#define FT_LOCAL_DEF( x )  static  x

#else

#ifdef __cplusplus
#define FT_LOCAL( x )      extern "C"  x
#define FT_LOCAL_DEF( x )  extern "C"  x
#else
#define FT_LOCAL( x )      extern  x
#define FT_LOCAL_DEF( x )  x
#endif

#endif /* FT_MAKE_OPTION_SINGLE_OBJECT */

#ifndef FT_BASE

#ifdef __cplusplus
#define FT_BASE( x )  extern "C"  x
#else
#define FT_BASE( x )  extern  x
#endif

#endif /* !FT_BASE */

#ifndef FT_BASE_DEF

#ifdef __cplusplus
#define FT_BASE_DEF( x )  x
#else
#define FT_BASE_DEF( x )  x
#endif

#endif /* !FT_BASE_DEF */

#ifndef FT_EXPORT

#ifdef __cplusplus
#define FT_EXPORT( x )  extern "C"  x
#else
#define FT_EXPORT( x )  extern  x
#endif

#endif /* !FT_EXPORT */

#ifndef FT_EXPORT_DEF

#ifdef __cplusplus
#define FT_EXPORT_DEF( x )  extern "C"  x
#else
#define FT_EXPORT_DEF( x )  extern  x
#endif

#endif /* !FT_EXPORT_DEF */

#ifndef FT_EXPORT_VAR

#ifdef __cplusplus
#define FT_EXPORT_VAR( x )  extern "C"  x
#else
#define FT_EXPORT_VAR( x )  extern  x
#endif

#endif /* !FT_EXPORT_VAR */

  /* The following macros are needed to compile the library with a   */
  /* C++ compiler and with 16bit compilers.                          */
  /*                                                                 */

  /* This is special.  Within C++, you must specify `extern "C"' for */
  /* functions which are used via function pointers, and you also    */
  /* must do that for structures which contain function pointers to  */
  /* assure C linkage -- it's not possible to have (local) anonymous */
  /* functions which are accessed by (global) function pointers.     */
  /*                                                                 */
  /*                                                                 */
  /* FT_CALLBACK_DEF is used to _define_ a callback function.        */
  /*                                                                 */
  /* FT_CALLBACK_TABLE is used to _declare_ a constant variable that */
  /* contains pointers to callback functions.                        */
  /*                                                                 */
  /* FT_CALLBACK_TABLE_DEF is used to _define_ a constant variable   */
  /* that contains pointers to callback functions.                   */
  /*                                                                 */
  /*                                                                 */
  /* Some 16bit compilers have to redefine these macros to insert    */
  /* the infamous `_cdecl' or `__fastcall' declarations.             */
  /*                                                                 */
#ifndef FT_CALLBACK_DEF
#ifdef __cplusplus
#define FT_CALLBACK_DEF( x )  extern "C"  x
#else
#define FT_CALLBACK_DEF( x )  static  x
#endif
#endif /* FT_CALLBACK_DEF */

#ifndef FT_CALLBACK_TABLE
#ifdef __cplusplus
#define FT_CALLBACK_TABLE      extern "C"
#define FT_CALLBACK_TABLE_DEF  extern "C"
#else
#define FT_CALLBACK_TABLE      extern
#define FT_CALLBACK_TABLE_DEF  /* nothing */
#endif
#endif /* FT_CALLBACK_TABLE */

FT_END_HEADER

#endif /* __FTCONFIG_H__ */

/* END */

/*** End of inlined file: ftconfig.h ***/


/*** Start of inlined file: fterrors.h ***/
/*                                                                         */
/*  fterrors.h                                                             */
/*                                                                         */
/*    FreeType error code handling (specification).                        */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2004, 2007 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This special header file is used to define the handling of FT2        */
  /* enumeration constants.  It can also be used to generate error message */
  /* strings with a small macro trick explained below.                     */
  /*                                                                       */
  /* I - Error Formats                                                     */
  /* -----------------                                                     */
  /*                                                                       */
  /*   The configuration macro FT_CONFIG_OPTION_USE_MODULE_ERRORS can be   */
  /*   defined in ftoption.h in order to make the higher byte indicate     */
  /*   the module where the error has happened (this is not compatible     */
  /*   with standard builds of FreeType 2).  You can then use the macro    */
  /*   FT_ERROR_BASE macro to extract the generic error code from an       */
  /*   FT_Error value.                                                     */
  /*                                                                       */
  /*                                                                       */
  /* II - Error Message strings                                            */
  /* --------------------------                                            */
  /*                                                                       */
  /*   The error definitions below are made through special macros that    */
  /*   allow client applications to build a table of error message strings */
  /*   if they need it.  The strings are not included in a normal build of */
  /*   FreeType 2 to save space (most client applications do not use       */
  /*   them).                                                              */
  /*                                                                       */
  /*   To do so, you have to define the following macros before including  */
  /*   this file:                                                          */
  /*                                                                       */
  /*   FT_ERROR_START_LIST ::                                              */
  /*     This macro is called before anything else to define the start of  */
  /*     the error list.  It is followed by several FT_ERROR_DEF calls     */
  /*     (see below).                                                      */
  /*                                                                       */
  /*   FT_ERROR_DEF( e, v, s ) ::                                          */
  /*     This macro is called to define one single error.                  */
  /*     `e' is the error code identifier (e.g. FT_Err_Invalid_Argument).  */
  /*     `v' is the error numerical value.                                 */
  /*     `s' is the corresponding error string.                            */
  /*                                                                       */
  /*   FT_ERROR_END_LIST ::                                                */
  /*     This macro ends the list.                                         */
  /*                                                                       */
  /*   Additionally, you have to undefine __FTERRORS_H__ before #including */
  /*   this file.                                                          */
  /*                                                                       */
  /*   Here is a simple example:                                           */
  /*                                                                       */
  /*     {                                                                 */
  /*       #undef __FTERRORS_H__                                           */
  /*       #define FT_ERRORDEF( e, v, s )  { e, s },                       */
  /*       #define FT_ERROR_START_LIST     {                               */
  /*       #define FT_ERROR_END_LIST       { 0, 0 } };                     */
  /*                                                                       */
  /*       const struct                                                    */
  /*       {                                                               */
  /*         int          err_code;                                        */
  /*         const char*  err_msg;                                         */
  /*       } ft_errors[] =                                                 */
  /*                                                                       */
  /*       #include FT_ERRORS_H                                            */
  /*     }                                                                 */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FTERRORS_H__
#define __FTERRORS_H__

  /* include module base error codes */

/*** Start of inlined file: ftmoderr.h ***/
/*                                                                         */
/*  ftmoderr.h                                                             */
/*                                                                         */
/*    FreeType module error offsets (specification).                       */
/*                                                                         */
/*  Copyright 2001, 2002, 2003, 2004, 2005, 2010 by                        */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file is used to define the FreeType module error offsets.        */
  /*                                                                       */
  /* The lower byte gives the error code, the higher byte gives the        */
  /* module.  The base module has error offset 0.  For example, the error  */
  /* `FT_Err_Invalid_File_Format' has value 0x003, the error               */
  /* `TT_Err_Invalid_File_Format' has value 0x1103, the error              */
  /* `T1_Err_Invalid_File_Format' has value 0x1203, etc.                   */
  /*                                                                       */
  /* Undefine the macro FT_CONFIG_OPTION_USE_MODULE_ERRORS in ftoption.h   */
  /* to make the higher byte always zero (disabling the module error       */
  /* mechanism).                                                           */
  /*                                                                       */
  /* It can also be used to create a module error message table easily     */
  /* with something like                                                   */
  /*                                                                       */
  /*   {                                                                   */
  /*     #undef __FTMODERR_H__                                             */
  /*     #define FT_MODERRDEF( e, v, s )  { FT_Mod_Err_ ## e, s },         */
  /*     #define FT_MODERR_START_LIST     {                                */
  /*     #define FT_MODERR_END_LIST       { 0, 0 } };                      */
  /*                                                                       */
  /*     const struct                                                      */
  /*     {                                                                 */
  /*       int          mod_err_offset;                                    */
  /*       const char*  mod_err_msg                                        */
  /*     } ft_mod_errors[] =                                               */
  /*                                                                       */
  /*     #include FT_MODULE_ERRORS_H                                       */
  /*   }                                                                   */
  /*                                                                       */
  /* To use such a table, all errors must be ANDed with 0xFF00 to remove   */
  /* the error code.                                                       */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FTMODERR_H__
#define __FTMODERR_H__

  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                       SETUP MACROS                      *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/

#undef  FT_NEED_EXTERN_C

#ifndef FT_MODERRDEF

#ifdef FT_CONFIG_OPTION_USE_MODULE_ERRORS
#define FT_MODERRDEF( e, v, s )  FT_Mod_Err_ ## e = v,
#else
#define FT_MODERRDEF( e, v, s )  FT_Mod_Err_ ## e = 0,
#endif

#define FT_MODERR_START_LIST  enum {
#define FT_MODERR_END_LIST    FT_Mod_Err_Max };

#ifdef __cplusplus
#define FT_NEED_EXTERN_C
  extern "C" {
#endif

#endif /* !FT_MODERRDEF */

  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****               LIST MODULE ERROR BASES                   *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/

#ifdef FT_MODERR_START_LIST
  FT_MODERR_START_LIST
#endif

  FT_MODERRDEF( Base,      0x000, "base module" )
  FT_MODERRDEF( Autofit,   0x100, "autofitter module" )
  FT_MODERRDEF( BDF,       0x200, "BDF module" )
  FT_MODERRDEF( Bzip2,     0x300, "Bzip2 module" )
  FT_MODERRDEF( Cache,     0x400, "cache module" )
  FT_MODERRDEF( CFF,       0x500, "CFF module" )
  FT_MODERRDEF( CID,       0x600, "CID module" )
  FT_MODERRDEF( Gzip,      0x700, "Gzip module" )
  FT_MODERRDEF( LZW,       0x800, "LZW module" )
  FT_MODERRDEF( OTvalid,   0x900, "OpenType validation module" )
  FT_MODERRDEF( PCF,       0xA00, "PCF module" )
  FT_MODERRDEF( PFR,       0xB00, "PFR module" )
  FT_MODERRDEF( PSaux,     0xC00, "PS auxiliary module" )
  FT_MODERRDEF( PShinter,  0xD00, "PS hinter module" )
  FT_MODERRDEF( PSnames,   0xE00, "PS names module" )
  FT_MODERRDEF( Raster,    0xF00, "raster module" )
  FT_MODERRDEF( SFNT,     0x1000, "SFNT module" )
  FT_MODERRDEF( Smooth,   0x1100, "smooth raster module" )
  FT_MODERRDEF( TrueType, 0x1200, "TrueType module" )
  FT_MODERRDEF( Type1,    0x1300, "Type 1 module" )
  FT_MODERRDEF( Type42,   0x1400, "Type 42 module" )
  FT_MODERRDEF( Winfonts, 0x1500, "Windows FON/FNT module" )

#ifdef FT_MODERR_END_LIST
  FT_MODERR_END_LIST
#endif

  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                      CLEANUP                            *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/

#ifdef FT_NEED_EXTERN_C
  }
#endif

#undef FT_MODERR_START_LIST
#undef FT_MODERR_END_LIST
#undef FT_MODERRDEF
#undef FT_NEED_EXTERN_C

#endif /* __FTMODERR_H__ */

/* END */

/*** End of inlined file: ftmoderr.h ***/


  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                       SETUP MACROS                      *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/

#undef  FT_NEED_EXTERN_C

#undef  FT_ERR_XCAT
#undef  FT_ERR_CAT

#define FT_ERR_XCAT( x, y )  x ## y
#define FT_ERR_CAT( x, y )   FT_ERR_XCAT( x, y )

  /* FT_ERR_PREFIX is used as a prefix for error identifiers. */
  /* By default, we use `FT_Err_'.                            */
  /*                                                          */
#ifndef FT_ERR_PREFIX
#define FT_ERR_PREFIX  FT_Err_
#endif

  /* FT_ERR_BASE is used as the base for module-specific errors. */
  /*                                                             */
#ifdef FT_CONFIG_OPTION_USE_MODULE_ERRORS

#ifndef FT_ERR_BASE
#define FT_ERR_BASE  FT_Mod_Err_Base
#endif

#else

#undef FT_ERR_BASE
#define FT_ERR_BASE  0

#endif /* FT_CONFIG_OPTION_USE_MODULE_ERRORS */

  /* If FT_ERRORDEF is not defined, we need to define a simple */
  /* enumeration type.                                         */
  /*                                                           */
#ifndef FT_ERRORDEF

#define FT_ERRORDEF( e, v, s )  e = v,
#define FT_ERROR_START_LIST     enum {
#define FT_ERROR_END_LIST       FT_ERR_CAT( FT_ERR_PREFIX, Max ) };

#ifdef __cplusplus
#define FT_NEED_EXTERN_C
  extern "C" {
#endif

#endif /* !FT_ERRORDEF */

  /* this macro is used to define an error */
#define FT_ERRORDEF_( e, v, s )   \
		  FT_ERRORDEF( FT_ERR_CAT( FT_ERR_PREFIX, e ), v + FT_ERR_BASE, s )

  /* this is only used for <module>_Err_Ok, which must be 0! */
#define FT_NOERRORDEF_( e, v, s ) \
		  FT_ERRORDEF( FT_ERR_CAT( FT_ERR_PREFIX, e ), v, s )

#ifdef FT_ERROR_START_LIST
  FT_ERROR_START_LIST
#endif

  /* now include the error codes */

/*** Start of inlined file: fterrdef.h ***/
/*                                                                         */
/*  fterrdef.h                                                             */
/*                                                                         */
/*    FreeType error codes (specification).                                */
/*                                                                         */
/*  Copyright 2002, 2004, 2006, 2007, 2010-2011 by                         */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                LIST OF ERROR CODES/MESSAGES             *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/

  /* You need to define both FT_ERRORDEF_ and FT_NOERRORDEF_ before */
  /* including this file.                                           */

  /* generic errors */

  FT_NOERRORDEF_( Ok,                                        0x00, \
				  "no error" )

  FT_ERRORDEF_( Cannot_Open_Resource,                        0x01, \
				"cannot open resource" )
  FT_ERRORDEF_( Unknown_File_Format,                         0x02, \
				"unknown file format" )
  FT_ERRORDEF_( Invalid_File_Format,                         0x03, \
				"broken file" )
  FT_ERRORDEF_( Invalid_Version,                             0x04, \
				"invalid FreeType version" )
  FT_ERRORDEF_( Lower_Module_Version,                        0x05, \
				"module version is too low" )
  FT_ERRORDEF_( Invalid_Argument,                            0x06, \
				"invalid argument" )
  FT_ERRORDEF_( Unimplemented_Feature,                       0x07, \
				"unimplemented feature" )
  FT_ERRORDEF_( Invalid_Table,                               0x08, \
				"broken table" )
  FT_ERRORDEF_( Invalid_Offset,                              0x09, \
				"broken offset within table" )
  FT_ERRORDEF_( Array_Too_Large,                             0x0A, \
				"array allocation size too large" )
  FT_ERRORDEF_( Missing_Module,                              0x0B, \
				"missing module" )

  /* glyph/character errors */

  FT_ERRORDEF_( Invalid_Glyph_Index,                         0x10, \
				"invalid glyph index" )
  FT_ERRORDEF_( Invalid_Character_Code,                      0x11, \
				"invalid character code" )
  FT_ERRORDEF_( Invalid_Glyph_Format,                        0x12, \
				"unsupported glyph image format" )
  FT_ERRORDEF_( Cannot_Render_Glyph,                         0x13, \
				"cannot render this glyph format" )
  FT_ERRORDEF_( Invalid_Outline,                             0x14, \
				"invalid outline" )
  FT_ERRORDEF_( Invalid_Composite,                           0x15, \
				"invalid composite glyph" )
  FT_ERRORDEF_( Too_Many_Hints,                              0x16, \
				"too many hints" )
  FT_ERRORDEF_( Invalid_Pixel_Size,                          0x17, \
				"invalid pixel size" )

  /* handle errors */

  FT_ERRORDEF_( Invalid_Handle,                              0x20, \
				"invalid object handle" )
  FT_ERRORDEF_( Invalid_Library_Handle,                      0x21, \
				"invalid library handle" )
  FT_ERRORDEF_( Invalid_Driver_Handle,                       0x22, \
				"invalid module handle" )
  FT_ERRORDEF_( Invalid_Face_Handle,                         0x23, \
				"invalid face handle" )
  FT_ERRORDEF_( Invalid_Size_Handle,                         0x24, \
				"invalid size handle" )
  FT_ERRORDEF_( Invalid_Slot_Handle,                         0x25, \
				"invalid glyph slot handle" )
  FT_ERRORDEF_( Invalid_CharMap_Handle,                      0x26, \
				"invalid charmap handle" )
  FT_ERRORDEF_( Invalid_Cache_Handle,                        0x27, \
				"invalid cache manager handle" )
  FT_ERRORDEF_( Invalid_Stream_Handle,                       0x28, \
				"invalid stream handle" )

  /* driver errors */

  FT_ERRORDEF_( Too_Many_Drivers,                            0x30, \
				"too many modules" )
  FT_ERRORDEF_( Too_Many_Extensions,                         0x31, \
				"too many extensions" )

  /* memory errors */

  FT_ERRORDEF_( Out_Of_Memory,                               0x40, \
				"out of memory" )
  FT_ERRORDEF_( Unlisted_Object,                             0x41, \
				"unlisted object" )

  /* stream errors */

  FT_ERRORDEF_( Cannot_Open_Stream,                          0x51, \
				"cannot open stream" )
  FT_ERRORDEF_( Invalid_Stream_Seek,                         0x52, \
				"invalid stream seek" )
  FT_ERRORDEF_( Invalid_Stream_Skip,                         0x53, \
				"invalid stream skip" )
  FT_ERRORDEF_( Invalid_Stream_Read,                         0x54, \
				"invalid stream read" )
  FT_ERRORDEF_( Invalid_Stream_Operation,                    0x55, \
				"invalid stream operation" )
  FT_ERRORDEF_( Invalid_Frame_Operation,                     0x56, \
				"invalid frame operation" )
  FT_ERRORDEF_( Nested_Frame_Access,                         0x57, \
				"nested frame access" )
  FT_ERRORDEF_( Invalid_Frame_Read,                          0x58, \
				"invalid frame read" )

  /* raster errors */

  FT_ERRORDEF_( Raster_Uninitialized,                        0x60, \
				"raster uninitialized" )
  FT_ERRORDEF_( Raster_Corrupted,                            0x61, \
				"raster corrupted" )
  FT_ERRORDEF_( Raster_Overflow,                             0x62, \
				"raster overflow" )
  FT_ERRORDEF_( Raster_Negative_Height,                      0x63, \
				"negative height while rastering" )

  /* cache errors */

  FT_ERRORDEF_( Too_Many_Caches,                             0x70, \
				"too many registered caches" )

  /* TrueType and SFNT errors */

  FT_ERRORDEF_( Invalid_Opcode,                              0x80, \
				"invalid opcode" )
  FT_ERRORDEF_( Too_Few_Arguments,                           0x81, \
				"too few arguments" )
  FT_ERRORDEF_( Stack_Overflow,                              0x82, \
				"stack overflow" )
  FT_ERRORDEF_( Code_Overflow,                               0x83, \
				"code overflow" )
  FT_ERRORDEF_( Bad_Argument,                                0x84, \
				"bad argument" )
  FT_ERRORDEF_( Divide_By_Zero,                              0x85, \
				"division by zero" )
  FT_ERRORDEF_( Invalid_Reference,                           0x86, \
				"invalid reference" )
  FT_ERRORDEF_( Debug_OpCode,                                0x87, \
				"found debug opcode" )
  FT_ERRORDEF_( ENDF_In_Exec_Stream,                         0x88, \
				"found ENDF opcode in execution stream" )
  FT_ERRORDEF_( Nested_DEFS,                                 0x89, \
				"nested DEFS" )
  FT_ERRORDEF_( Invalid_CodeRange,                           0x8A, \
				"invalid code range" )
  FT_ERRORDEF_( Execution_Too_Long,                          0x8B, \
				"execution context too long" )
  FT_ERRORDEF_( Too_Many_Function_Defs,                      0x8C, \
				"too many function definitions" )
  FT_ERRORDEF_( Too_Many_Instruction_Defs,                   0x8D, \
				"too many instruction definitions" )
  FT_ERRORDEF_( Table_Missing,                               0x8E, \
				"SFNT font table missing" )
  FT_ERRORDEF_( Horiz_Header_Missing,                        0x8F, \
				"horizontal header (hhea) table missing" )
  FT_ERRORDEF_( Locations_Missing,                           0x90, \
				"locations (loca) table missing" )
  FT_ERRORDEF_( Name_Table_Missing,                          0x91, \
				"name table missing" )
  FT_ERRORDEF_( CMap_Table_Missing,                          0x92, \
				"character map (cmap) table missing" )
  FT_ERRORDEF_( Hmtx_Table_Missing,                          0x93, \
				"horizontal metrics (hmtx) table missing" )
  FT_ERRORDEF_( Post_Table_Missing,                          0x94, \
				"PostScript (post) table missing" )
  FT_ERRORDEF_( Invalid_Horiz_Metrics,                       0x95, \
				"invalid horizontal metrics" )
  FT_ERRORDEF_( Invalid_CharMap_Format,                      0x96, \
				"invalid character map (cmap) format" )
  FT_ERRORDEF_( Invalid_PPem,                                0x97, \
				"invalid ppem value" )
  FT_ERRORDEF_( Invalid_Vert_Metrics,                        0x98, \
				"invalid vertical metrics" )
  FT_ERRORDEF_( Could_Not_Find_Context,                      0x99, \
				"could not find context" )
  FT_ERRORDEF_( Invalid_Post_Table_Format,                   0x9A, \
				"invalid PostScript (post) table format" )
  FT_ERRORDEF_( Invalid_Post_Table,                          0x9B, \
				"invalid PostScript (post) table" )

  /* CFF, CID, and Type 1 errors */

  FT_ERRORDEF_( Syntax_Error,                                0xA0, \
				"opcode syntax error" )
  FT_ERRORDEF_( Stack_Underflow,                             0xA1, \
				"argument stack underflow" )
  FT_ERRORDEF_( Ignore,                                      0xA2, \
				"ignore" )
  FT_ERRORDEF_( No_Unicode_Glyph_Name,                       0xA3, \
				"no Unicode glyph name found" )

  /* BDF errors */

  FT_ERRORDEF_( Missing_Startfont_Field,                     0xB0, \
				"`STARTFONT' field missing" )
  FT_ERRORDEF_( Missing_Font_Field,                          0xB1, \
				"`FONT' field missing" )
  FT_ERRORDEF_( Missing_Size_Field,                          0xB2, \
				"`SIZE' field missing" )
  FT_ERRORDEF_( Missing_Fontboundingbox_Field,               0xB3, \
				"`FONTBOUNDINGBOX' field missing" )
  FT_ERRORDEF_( Missing_Chars_Field,                         0xB4, \
				"`CHARS' field missing" )
  FT_ERRORDEF_( Missing_Startchar_Field,                     0xB5, \
				"`STARTCHAR' field missing" )
  FT_ERRORDEF_( Missing_Encoding_Field,                      0xB6, \
				"`ENCODING' field missing" )
  FT_ERRORDEF_( Missing_Bbx_Field,                           0xB7, \
				"`BBX' field missing" )
  FT_ERRORDEF_( Bbx_Too_Big,                                 0xB8, \
				"`BBX' too big" )
  FT_ERRORDEF_( Corrupted_Font_Header,                       0xB9, \
				"Font header corrupted or missing fields" )
  FT_ERRORDEF_( Corrupted_Font_Glyphs,                       0xBA, \
				"Font glyphs corrupted or missing fields" )

/* END */

/*** End of inlined file: fterrdef.h ***/


#ifdef FT_ERROR_END_LIST
  FT_ERROR_END_LIST
#endif

  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                      SIMPLE CLEANUP                     *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/

#ifdef FT_NEED_EXTERN_C
  }
#endif

#undef FT_ERROR_START_LIST
#undef FT_ERROR_END_LIST

#undef FT_ERRORDEF
#undef FT_ERRORDEF_
#undef FT_NOERRORDEF_

#undef FT_NEED_EXTERN_C
#undef FT_ERR_BASE

  /* FT_KEEP_ERR_PREFIX is needed for ftvalid.h */
#ifndef FT_KEEP_ERR_PREFIX
#undef FT_ERR_PREFIX
#else
#undef FT_KEEP_ERR_PREFIX
#endif

#endif /* __FTERRORS_H__ */

/* END */

/*** End of inlined file: fterrors.h ***/


/*** Start of inlined file: fttypes.h ***/
/*                                                                         */
/*  fttypes.h                                                              */
/*                                                                         */
/*    FreeType simple types definitions (specification only).              */
/*                                                                         */
/*  Copyright 1996-2002, 2004, 2006-2009, 2012 by                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FTTYPES_H__
#define __FTTYPES_H__


/*** Start of inlined file: ftsystem.h ***/
/*                                                                         */
/*  ftsystem.h                                                             */
/*                                                                         */
/*    FreeType low-level system interface definition (specification).      */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2005, 2010 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FTSYSTEM_H__
#define __FTSYSTEM_H__

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*   system_interface                                                    */
  /*                                                                       */
  /* <Title>                                                               */
  /*   System Interface                                                    */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*   How FreeType manages memory and i/o.                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*   This section contains various definitions related to memory         */
  /*   management and i/o access.  You need to understand this             */
  /*   information if you want to use a custom memory manager or you own   */
  /*   i/o streams.                                                        */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /*                  M E M O R Y   M A N A G E M E N T                    */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************
   *
   * @type:
   *   FT_Memory
   *
   * @description:
   *   A handle to a given memory manager object, defined with an
   *   @FT_MemoryRec structure.
   *
   */
  typedef struct FT_MemoryRec_*  FT_Memory;

  /*************************************************************************
   *
   * @functype:
   *   FT_Alloc_Func
   *
   * @description:
   *   A function used to allocate `size' bytes from `memory'.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   size ::
   *     The size in bytes to allocate.
   *
   * @return:
   *   Address of new memory block.  0~in case of failure.
   *
   */
  typedef void*
  (*FT_Alloc_Func)( FT_Memory  memory,
					long       size );

  /*************************************************************************
   *
   * @functype:
   *   FT_Free_Func
   *
   * @description:
   *   A function used to release a given block of memory.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   block ::
   *     The address of the target memory block.
   *
   */
  typedef void
  (*FT_Free_Func)( FT_Memory  memory,
				   void*      block );

  /*************************************************************************
   *
   * @functype:
   *   FT_Realloc_Func
   *
   * @description:
   *   A function used to re-allocate a given block of memory.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   cur_size ::
   *     The block's current size in bytes.
   *
   *   new_size ::
   *     The block's requested new size.
   *
   *   block ::
   *     The block's current address.
   *
   * @return:
   *   New block address.  0~in case of memory shortage.
   *
   * @note:
   *   In case of error, the old block must still be available.
   *
   */
  typedef void*
  (*FT_Realloc_Func)( FT_Memory  memory,
					  long       cur_size,
					  long       new_size,
					  void*      block );

  /*************************************************************************
   *
   * @struct:
   *   FT_MemoryRec
   *
   * @description:
   *   A structure used to describe a given memory manager to FreeType~2.
   *
   * @fields:
   *   user ::
   *     A generic typeless pointer for user data.
   *
   *   alloc ::
   *     A pointer type to an allocation function.
   *
   *   free ::
   *     A pointer type to an memory freeing function.
   *
   *   realloc ::
   *     A pointer type to a reallocation function.
   *
   */
  struct  FT_MemoryRec_
  {
	void*            user;
	FT_Alloc_Func    alloc;
	FT_Free_Func     free;
	FT_Realloc_Func  realloc;
  };

  /*************************************************************************/
  /*                                                                       */
  /*                       I / O   M A N A G E M E N T                     */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************
   *
   * @type:
   *   FT_Stream
   *
   * @description:
   *   A handle to an input stream.
   *
   */
  typedef struct FT_StreamRec_*  FT_Stream;

  /*************************************************************************
   *
   * @struct:
   *   FT_StreamDesc
   *
   * @description:
   *   A union type used to store either a long or a pointer.  This is used
   *   to store a file descriptor or a `FILE*' in an input stream.
   *
   */
  typedef union  FT_StreamDesc_
  {
	long   value;
	void*  pointer;

  } FT_StreamDesc;

  /*************************************************************************
   *
   * @functype:
   *   FT_Stream_IoFunc
   *
   * @description:
   *   A function used to seek and read data from a given input stream.
   *
   * @input:
   *   stream ::
   *     A handle to the source stream.
   *
   *   offset ::
   *     The offset of read in stream (always from start).
   *
   *   buffer ::
   *     The address of the read buffer.
   *
   *   count ::
   *     The number of bytes to read from the stream.
   *
   * @return:
   *   The number of bytes effectively read by the stream.
   *
   * @note:
   *   This function might be called to perform a seek or skip operation
   *   with a `count' of~0.  A non-zero return value then indicates an
   *   error.
   *
   */
  typedef unsigned long
  (*FT_Stream_IoFunc)( FT_Stream       stream,
					   unsigned long   offset,
					   unsigned char*  buffer,
					   unsigned long   count );

  /*************************************************************************
   *
   * @functype:
   *   FT_Stream_CloseFunc
   *
   * @description:
   *   A function used to close a given input stream.
   *
   * @input:
   *  stream ::
   *     A handle to the target stream.
   *
   */
  typedef void
  (*FT_Stream_CloseFunc)( FT_Stream  stream );

  /*************************************************************************
   *
   * @struct:
   *   FT_StreamRec
   *
   * @description:
   *   A structure used to describe an input stream.
   *
   * @input:
   *   base ::
   *     For memory-based streams, this is the address of the first stream
   *     byte in memory.  This field should always be set to NULL for
   *     disk-based streams.
   *
   *   size ::
   *     The stream size in bytes.
   *
   *   pos ::
   *     The current position within the stream.
   *
   *   descriptor ::
   *     This field is a union that can hold an integer or a pointer.  It is
   *     used by stream implementations to store file descriptors or `FILE*'
   *     pointers.
   *
   *   pathname ::
   *     This field is completely ignored by FreeType.  However, it is often
   *     useful during debugging to use it to store the stream's filename
   *     (where available).
   *
   *   read ::
   *     The stream's input function.
   *
   *   close ::
   *     The stream's close function.
   *
   *   memory ::
   *     The memory manager to use to preload frames.  This is set
   *     internally by FreeType and shouldn't be touched by stream
   *     implementations.
   *
   *   cursor ::
   *     This field is set and used internally by FreeType when parsing
   *     frames.
   *
   *   limit ::
   *     This field is set and used internally by FreeType when parsing
   *     frames.
   *
   */
  typedef struct  FT_StreamRec_
  {
	unsigned char*       base;
	unsigned long        size;
	unsigned long        pos;

	FT_StreamDesc        descriptor;
	FT_StreamDesc        pathname;
	FT_Stream_IoFunc     read;
	FT_Stream_CloseFunc  close;

	FT_Memory            memory;
	unsigned char*       cursor;
	unsigned char*       limit;

  } FT_StreamRec;

  /* */

FT_END_HEADER

#endif /* __FTSYSTEM_H__ */

/* END */

/*** End of inlined file: ftsystem.h ***/


/*** Start of inlined file: ftimage.h ***/
/*                                                                         */
/*  ftimage.h                                                              */
/*                                                                         */
/*    FreeType glyph image formats and default raster interface            */
/*    (specification).                                                     */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,   */
/*            2010 by                                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* Note: A `raster' is simply a scan-line converter, used to render      */
  /*       FT_Outlines into FT_Bitmaps.                                    */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FTIMAGE_H__
#define __FTIMAGE_H__

  /* _STANDALONE_ is from ftgrays.c */
#ifndef _STANDALONE_

#endif

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    basic_types                                                        */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Pos                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The type FT_Pos is used to store vectorial coordinates.  Depending */
  /*    on the context, these can represent distances in integer font      */
  /*    units, or 16.16, or 26.6 fixed float pixel coordinates.            */
  /*                                                                       */
  typedef signed long  FT_Pos;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Vector                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2D vector; coordinates are of   */
  /*    the FT_Pos type.                                                   */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x :: The horizontal coordinate.                                    */
  /*    y :: The vertical coordinate.                                      */
  /*                                                                       */
  typedef struct  FT_Vector_
  {
	FT_Pos  x;
	FT_Pos  y;

  } FT_Vector;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_BBox                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold an outline's bounding box, i.e., the      */
  /*    coordinates of its extrema in the horizontal and vertical          */
  /*    directions.                                                        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    xMin :: The horizontal minimum (left-most).                        */
  /*                                                                       */
  /*    yMin :: The vertical minimum (bottom-most).                        */
  /*                                                                       */
  /*    xMax :: The horizontal maximum (right-most).                       */
  /*                                                                       */
  /*    yMax :: The vertical maximum (top-most).                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The bounding box is specified with the coordinates of the lower    */
  /*    left and the upper right corner.  In PostScript, those values are  */
  /*    often called (llx,lly) and (urx,ury), respectively.                */
  /*                                                                       */
  /*    If `yMin' is negative, this value gives the glyph's descender.     */
  /*    Otherwise, the glyph doesn't descend below the baseline.           */
  /*    Similarly, if `ymax' is positive, this value gives the glyph's     */
  /*    ascender.                                                          */
  /*                                                                       */
  /*    `xMin' gives the horizontal distance from the glyph's origin to    */
  /*    the left edge of the glyph's bounding box.  If `xMin' is negative, */
  /*    the glyph extends to the left of the origin.                       */
  /*                                                                       */
  typedef struct  FT_BBox_
  {
	FT_Pos  xMin, yMin;
	FT_Pos  xMax, yMax;

  } FT_BBox;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Pixel_Mode                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration type used to describe the format of pixels in a     */
  /*    given bitmap.  Note that additional formats may be added in the    */
  /*    future.                                                            */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_PIXEL_MODE_NONE ::                                              */
  /*      Value~0 is reserved.                                             */
  /*                                                                       */
  /*    FT_PIXEL_MODE_MONO ::                                              */
  /*      A monochrome bitmap, using 1~bit per pixel.  Note that pixels    */
  /*      are stored in most-significant order (MSB), which means that     */
  /*      the left-most pixel in a byte has value 128.                     */
  /*                                                                       */
  /*    FT_PIXEL_MODE_GRAY ::                                              */
  /*      An 8-bit bitmap, generally used to represent anti-aliased glyph  */
  /*      images.  Each pixel is stored in one byte.  Note that the number */
  /*      of `gray' levels is stored in the `num_grays' field of the       */
  /*      @FT_Bitmap structure (it generally is 256).                      */
  /*                                                                       */
  /*    FT_PIXEL_MODE_GRAY2 ::                                             */
  /*      A 2-bit per pixel bitmap, used to represent embedded             */
  /*      anti-aliased bitmaps in font files according to the OpenType     */
  /*      specification.  We haven't found a single font using this        */
  /*      format, however.                                                 */
  /*                                                                       */
  /*    FT_PIXEL_MODE_GRAY4 ::                                             */
  /*      A 4-bit per pixel bitmap, representing embedded anti-aliased     */
  /*      bitmaps in font files according to the OpenType specification.   */
  /*      We haven't found a single font using this format, however.       */
  /*                                                                       */
  /*    FT_PIXEL_MODE_LCD ::                                               */
  /*      An 8-bit bitmap, representing RGB or BGR decimated glyph images  */
  /*      used for display on LCD displays; the bitmap is three times      */
  /*      wider than the original glyph image.  See also                   */
  /*      @FT_RENDER_MODE_LCD.                                             */
  /*                                                                       */
  /*    FT_PIXEL_MODE_LCD_V ::                                             */
  /*      An 8-bit bitmap, representing RGB or BGR decimated glyph images  */
  /*      used for display on rotated LCD displays; the bitmap is three    */
  /*      times taller than the original glyph image.  See also            */
  /*      @FT_RENDER_MODE_LCD_V.                                           */
  /*                                                                       */
  typedef enum  FT_Pixel_Mode_
  {
	FT_PIXEL_MODE_NONE = 0,
	FT_PIXEL_MODE_MONO,
	FT_PIXEL_MODE_GRAY,
	FT_PIXEL_MODE_GRAY2,
	FT_PIXEL_MODE_GRAY4,
	FT_PIXEL_MODE_LCD,
	FT_PIXEL_MODE_LCD_V,

	FT_PIXEL_MODE_MAX      /* do not remove */

  } FT_Pixel_Mode;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    ft_pixel_mode_xxx                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A list of deprecated constants.  Use the corresponding             */
  /*    @FT_Pixel_Mode values instead.                                     */
  /*                                                                       */
  /* <Values>                                                              */
  /*    ft_pixel_mode_none  :: See @FT_PIXEL_MODE_NONE.                    */
  /*    ft_pixel_mode_mono  :: See @FT_PIXEL_MODE_MONO.                    */
  /*    ft_pixel_mode_grays :: See @FT_PIXEL_MODE_GRAY.                    */
  /*    ft_pixel_mode_pal2  :: See @FT_PIXEL_MODE_GRAY2.                   */
  /*    ft_pixel_mode_pal4  :: See @FT_PIXEL_MODE_GRAY4.                   */
  /*                                                                       */
#define ft_pixel_mode_none   FT_PIXEL_MODE_NONE
#define ft_pixel_mode_mono   FT_PIXEL_MODE_MONO
#define ft_pixel_mode_grays  FT_PIXEL_MODE_GRAY
#define ft_pixel_mode_pal2   FT_PIXEL_MODE_GRAY2
#define ft_pixel_mode_pal4   FT_PIXEL_MODE_GRAY4

 /* */

#if 0

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Palette_Mode                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    THIS TYPE IS DEPRECATED.  DO NOT USE IT!                           */
  /*                                                                       */
  /*    An enumeration type to describe the format of a bitmap palette,    */
  /*    used with ft_pixel_mode_pal4 and ft_pixel_mode_pal8.               */
  /*                                                                       */
  /* <Values>                                                              */
  /*    ft_palette_mode_rgb  :: The palette is an array of 3-byte RGB      */
  /*                            records.                                   */
  /*                                                                       */
  /*    ft_palette_mode_rgba :: The palette is an array of 4-byte RGBA     */
  /*                            records.                                   */
  /*                                                                       */
  /* <Note>                                                                */
  /*    As ft_pixel_mode_pal2, pal4 and pal8 are currently unused by       */
  /*    FreeType, these types are not handled by the library itself.       */
  /*                                                                       */
  typedef enum  FT_Palette_Mode_
  {
	ft_palette_mode_rgb = 0,
	ft_palette_mode_rgba,

	ft_palette_mode_max   /* do not remove */

  } FT_Palette_Mode;

  /* */

#endif

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Bitmap                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to describe a bitmap or pixmap to the raster.     */
  /*    Note that we now manage pixmaps of various depths through the      */
  /*    `pixel_mode' field.                                                */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    rows         :: The number of bitmap rows.                         */
  /*                                                                       */
  /*    width        :: The number of pixels in bitmap row.                */
  /*                                                                       */
  /*    pitch        :: The pitch's absolute value is the number of bytes  */
  /*                    taken by one bitmap row, including padding.        */
  /*                    However, the pitch is positive when the bitmap has */
  /*                    a `down' flow, and negative when it has an `up'    */
  /*                    flow.  In all cases, the pitch is an offset to add */
  /*                    to a bitmap pointer in order to go down one row.   */
  /*                                                                       */
  /*                    Note that `padding' means the alignment of a       */
  /*                    bitmap to a byte border, and FreeType functions    */
  /*                    normally align to the smallest possible integer    */
  /*                    value.                                             */
  /*                                                                       */
  /*                    For the B/W rasterizer, `pitch' is always an even  */
  /*                    number.                                            */
  /*                                                                       */
  /*                    To change the pitch of a bitmap (say, to make it a */
  /*                    multiple of 4), use @FT_Bitmap_Convert.            */
  /*                    Alternatively, you might use callback functions to */
  /*                    directly render to the application's surface; see  */
  /*                    the file `example2.cpp' in the tutorial for a      */
  /*                    demonstration.                                     */
  /*                                                                       */
  /*    buffer       :: A typeless pointer to the bitmap buffer.  This     */
  /*                    value should be aligned on 32-bit boundaries in    */
  /*                    most cases.                                        */
  /*                                                                       */
  /*    num_grays    :: This field is only used with                       */
  /*                    @FT_PIXEL_MODE_GRAY; it gives the number of gray   */
  /*                    levels used in the bitmap.                         */
  /*                                                                       */
  /*    pixel_mode   :: The pixel mode, i.e., how pixel bits are stored.   */
  /*                    See @FT_Pixel_Mode for possible values.            */
  /*                                                                       */
  /*    palette_mode :: This field is intended for paletted pixel modes;   */
  /*                    it indicates how the palette is stored.  Not       */
  /*                    used currently.                                    */
  /*                                                                       */
  /*    palette      :: A typeless pointer to the bitmap palette; this     */
  /*                    field is intended for paletted pixel modes.  Not   */
  /*                    used currently.                                    */
  /*                                                                       */
  /* <Note>                                                                */
  /*   For now, the only pixel modes supported by FreeType are mono and    */
  /*   grays.  However, drivers might be added in the future to support    */
  /*   more `colorful' options.                                            */
  /*                                                                       */
  typedef struct  FT_Bitmap_
  {
	int             rows;
	int             width;
	int             pitch;
	unsigned char*  buffer;
	short           num_grays;
	char            pixel_mode;
	char            palette_mode;
	void*           palette;

  } FT_Bitmap;

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    outline_processing                                                 */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Outline                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This structure is used to describe an outline to the scan-line     */
  /*    converter.                                                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    n_contours :: The number of contours in the outline.               */
  /*                                                                       */
  /*    n_points   :: The number of points in the outline.                 */
  /*                                                                       */
  /*    points     :: A pointer to an array of `n_points' @FT_Vector       */
  /*                  elements, giving the outline's point coordinates.    */
  /*                                                                       */
  /*    tags       :: A pointer to an array of `n_points' chars, giving    */
  /*                  each outline point's type.                           */
  /*                                                                       */
  /*                  If bit~0 is unset, the point is `off' the curve,     */
  /*                  i.e., a Bézier control point, while it is `on' if    */
  /*                  set.                                                 */
  /*                                                                       */
  /*                  Bit~1 is meaningful for `off' points only.  If set,  */
  /*                  it indicates a third-order Bézier arc control point; */
  /*                  and a second-order control point if unset.           */
  /*                                                                       */
  /*                  If bit~2 is set, bits 5-7 contain the drop-out mode  */
  /*                  (as defined in the OpenType specification; the value */
  /*                  is the same as the argument to the SCANMODE          */
  /*                  instruction).                                        */
  /*                                                                       */
  /*                  Bits 3 and~4 are reserved for internal purposes.     */
  /*                                                                       */
  /*    contours   :: An array of `n_contours' shorts, giving the end      */
  /*                  point of each contour within the outline.  For       */
  /*                  example, the first contour is defined by the points  */
  /*                  `0' to `contours[0]', the second one is defined by   */
  /*                  the points `contours[0]+1' to `contours[1]', etc.    */
  /*                                                                       */
  /*    flags      :: A set of bit flags used to characterize the outline  */
  /*                  and give hints to the scan-converter and hinter on   */
  /*                  how to convert/grid-fit it.  See @FT_OUTLINE_FLAGS.  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The B/W rasterizer only checks bit~2 in the `tags' array for the   */
  /*    first point of each contour.  The drop-out mode as given with      */
  /*    @FT_OUTLINE_IGNORE_DROPOUTS, @FT_OUTLINE_SMART_DROPOUTS, and       */
  /*    @FT_OUTLINE_INCLUDE_STUBS in `flags' is then overridden.           */
  /*                                                                       */
  typedef struct  FT_Outline_
  {
	short       n_contours;      /* number of contours in glyph        */
	short       n_points;        /* number of points in the glyph      */

	FT_Vector*  points;          /* the outline's points               */
	char*       tags;            /* the points flags                   */
	short*      contours;        /* the contour end points             */

	int         flags;           /* outline masks                      */

  } FT_Outline;

  /* Following limits must be consistent with */
  /* FT_Outline.{n_contours,n_points}         */
#define FT_OUTLINE_CONTOURS_MAX  SHRT_MAX
#define FT_OUTLINE_POINTS_MAX    SHRT_MAX

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_OUTLINE_FLAGS                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A list of bit-field constants use for the flags in an outline's    */
  /*    `flags' field.                                                     */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_OUTLINE_NONE ::                                                 */
  /*      Value~0 is reserved.                                             */
  /*                                                                       */
  /*    FT_OUTLINE_OWNER ::                                                */
  /*      If set, this flag indicates that the outline's field arrays      */
  /*      (i.e., `points', `flags', and `contours') are `owned' by the     */
  /*      outline object, and should thus be freed when it is destroyed.   */
  /*                                                                       */
  /*    FT_OUTLINE_EVEN_ODD_FILL ::                                        */
  /*      By default, outlines are filled using the non-zero winding rule. */
  /*      If set to 1, the outline will be filled using the even-odd fill  */
  /*      rule (only works with the smooth rasterizer).                    */
  /*                                                                       */
  /*    FT_OUTLINE_REVERSE_FILL ::                                         */
  /*      By default, outside contours of an outline are oriented in       */
  /*      clock-wise direction, as defined in the TrueType specification.  */
  /*      This flag is set if the outline uses the opposite direction      */
  /*      (typically for Type~1 fonts).  This flag is ignored by the scan  */
  /*      converter.                                                       */
  /*                                                                       */
  /*    FT_OUTLINE_IGNORE_DROPOUTS ::                                      */
  /*      By default, the scan converter will try to detect drop-outs in   */
  /*      an outline and correct the glyph bitmap to ensure consistent     */
  /*      shape continuity.  If set, this flag hints the scan-line         */
  /*      converter to ignore such cases.  See below for more information. */
  /*                                                                       */
  /*    FT_OUTLINE_SMART_DROPOUTS ::                                       */
  /*      Select smart dropout control.  If unset, use simple dropout      */
  /*      control.  Ignored if @FT_OUTLINE_IGNORE_DROPOUTS is set.  See    */
  /*      below for more information.                                      */
  /*                                                                       */
  /*    FT_OUTLINE_INCLUDE_STUBS ::                                        */
  /*      If set, turn pixels on for `stubs', otherwise exclude them.      */
  /*      Ignored if @FT_OUTLINE_IGNORE_DROPOUTS is set.  See below for    */
  /*      more information.                                                */
  /*                                                                       */
  /*    FT_OUTLINE_HIGH_PRECISION ::                                       */
  /*      This flag indicates that the scan-line converter should try to   */
  /*      convert this outline to bitmaps with the highest possible        */
  /*      quality.  It is typically set for small character sizes.  Note   */
  /*      that this is only a hint that might be completely ignored by a   */
  /*      given scan-converter.                                            */
  /*                                                                       */
  /*    FT_OUTLINE_SINGLE_PASS ::                                          */
  /*      This flag is set to force a given scan-converter to only use a   */
  /*      single pass over the outline to render a bitmap glyph image.     */
  /*      Normally, it is set for very large character sizes.  It is only  */
  /*      a hint that might be completely ignored by a given               */
  /*      scan-converter.                                                  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The flags @FT_OUTLINE_IGNORE_DROPOUTS, @FT_OUTLINE_SMART_DROPOUTS, */
  /*    and @FT_OUTLINE_INCLUDE_STUBS are ignored by the smooth            */
  /*    rasterizer.                                                        */
  /*                                                                       */
  /*    There exists a second mechanism to pass the drop-out mode to the   */
  /*    B/W rasterizer; see the `tags' field in @FT_Outline.               */
  /*                                                                       */
  /*    Please refer to the description of the `SCANTYPE' instruction in   */
  /*    the OpenType specification (in file `ttinst1.doc') how simple      */
  /*    drop-outs, smart drop-outs, and stubs are defined.                 */
  /*                                                                       */
#define FT_OUTLINE_NONE             0x0
#define FT_OUTLINE_OWNER            0x1
#define FT_OUTLINE_EVEN_ODD_FILL    0x2
#define FT_OUTLINE_REVERSE_FILL     0x4
#define FT_OUTLINE_IGNORE_DROPOUTS  0x8
#define FT_OUTLINE_SMART_DROPOUTS   0x10
#define FT_OUTLINE_INCLUDE_STUBS    0x20

#define FT_OUTLINE_HIGH_PRECISION   0x100
#define FT_OUTLINE_SINGLE_PASS      0x200

 /*************************************************************************
  *
  * @enum:
  *   ft_outline_flags
  *
  * @description:
  *   These constants are deprecated.  Please use the corresponding
  *   @FT_OUTLINE_FLAGS values.
  *
  * @values:
  *   ft_outline_none            :: See @FT_OUTLINE_NONE.
  *   ft_outline_owner           :: See @FT_OUTLINE_OWNER.
  *   ft_outline_even_odd_fill   :: See @FT_OUTLINE_EVEN_ODD_FILL.
  *   ft_outline_reverse_fill    :: See @FT_OUTLINE_REVERSE_FILL.
  *   ft_outline_ignore_dropouts :: See @FT_OUTLINE_IGNORE_DROPOUTS.
  *   ft_outline_high_precision  :: See @FT_OUTLINE_HIGH_PRECISION.
  *   ft_outline_single_pass     :: See @FT_OUTLINE_SINGLE_PASS.
  */
#define ft_outline_none             FT_OUTLINE_NONE
#define ft_outline_owner            FT_OUTLINE_OWNER
#define ft_outline_even_odd_fill    FT_OUTLINE_EVEN_ODD_FILL
#define ft_outline_reverse_fill     FT_OUTLINE_REVERSE_FILL
#define ft_outline_ignore_dropouts  FT_OUTLINE_IGNORE_DROPOUTS
#define ft_outline_high_precision   FT_OUTLINE_HIGH_PRECISION
#define ft_outline_single_pass      FT_OUTLINE_SINGLE_PASS

  /* */

#define FT_CURVE_TAG( flag )  ( flag & 3 )

#define FT_CURVE_TAG_ON            1
#define FT_CURVE_TAG_CONIC         0
#define FT_CURVE_TAG_CUBIC         2

#define FT_CURVE_TAG_HAS_SCANMODE  4

#define FT_CURVE_TAG_TOUCH_X       8  /* reserved for the TrueType hinter */
#define FT_CURVE_TAG_TOUCH_Y      16  /* reserved for the TrueType hinter */

#define FT_CURVE_TAG_TOUCH_BOTH    ( FT_CURVE_TAG_TOUCH_X | \
									 FT_CURVE_TAG_TOUCH_Y )

#define FT_Curve_Tag_On       FT_CURVE_TAG_ON
#define FT_Curve_Tag_Conic    FT_CURVE_TAG_CONIC
#define FT_Curve_Tag_Cubic    FT_CURVE_TAG_CUBIC
#define FT_Curve_Tag_Touch_X  FT_CURVE_TAG_TOUCH_X
#define FT_Curve_Tag_Touch_Y  FT_CURVE_TAG_TOUCH_Y

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Outline_MoveToFunc                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function pointer type used to describe the signature of a `move  */
  /*    to' function during outline walking/decomposition.                 */
  /*                                                                       */
  /*    A `move to' is emitted to start a new contour in an outline.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    to   :: A pointer to the target point of the `move to'.            */
  /*                                                                       */
  /*    user :: A typeless pointer which is passed from the caller of the  */
  /*            decomposition function.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0~means success.                                      */
  /*                                                                       */
  typedef int
  (*FT_Outline_MoveToFunc)( const FT_Vector*  to,
							void*             user );

#define FT_Outline_MoveTo_Func  FT_Outline_MoveToFunc

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Outline_LineToFunc                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function pointer type used to describe the signature of a `line  */
  /*    to' function during outline walking/decomposition.                 */
  /*                                                                       */
  /*    A `line to' is emitted to indicate a segment in the outline.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    to   :: A pointer to the target point of the `line to'.            */
  /*                                                                       */
  /*    user :: A typeless pointer which is passed from the caller of the  */
  /*            decomposition function.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0~means success.                                      */
  /*                                                                       */
  typedef int
  (*FT_Outline_LineToFunc)( const FT_Vector*  to,
							void*             user );

#define FT_Outline_LineTo_Func  FT_Outline_LineToFunc

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Outline_ConicToFunc                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function pointer type used to describe the signature of a `conic */
  /*    to' function during outline walking or decomposition.              */
  /*                                                                       */
  /*    A `conic to' is emitted to indicate a second-order Bézier arc in   */
  /*    the outline.                                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    control :: An intermediate control point between the last position */
  /*               and the new target in `to'.                             */
  /*                                                                       */
  /*    to      :: A pointer to the target end point of the conic arc.     */
  /*                                                                       */
  /*    user    :: A typeless pointer which is passed from the caller of   */
  /*               the decomposition function.                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0~means success.                                      */
  /*                                                                       */
  typedef int
  (*FT_Outline_ConicToFunc)( const FT_Vector*  control,
							 const FT_Vector*  to,
							 void*             user );

#define FT_Outline_ConicTo_Func  FT_Outline_ConicToFunc

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Outline_CubicToFunc                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function pointer type used to describe the signature of a `cubic */
  /*    to' function during outline walking or decomposition.              */
  /*                                                                       */
  /*    A `cubic to' is emitted to indicate a third-order Bézier arc.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    control1 :: A pointer to the first Bézier control point.           */
  /*                                                                       */
  /*    control2 :: A pointer to the second Bézier control point.          */
  /*                                                                       */
  /*    to       :: A pointer to the target end point.                     */
  /*                                                                       */
  /*    user     :: A typeless pointer which is passed from the caller of  */
  /*                the decomposition function.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0~means success.                                      */
  /*                                                                       */
  typedef int
  (*FT_Outline_CubicToFunc)( const FT_Vector*  control1,
							 const FT_Vector*  control2,
							 const FT_Vector*  to,
							 void*             user );

#define FT_Outline_CubicTo_Func  FT_Outline_CubicToFunc

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Outline_Funcs                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure to hold various function pointers used during outline  */
  /*    decomposition in order to emit segments, conic, and cubic Béziers. */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    move_to  :: The `move to' emitter.                                 */
  /*                                                                       */
  /*    line_to  :: The segment emitter.                                   */
  /*                                                                       */
  /*    conic_to :: The second-order Bézier arc emitter.                   */
  /*                                                                       */
  /*    cubic_to :: The third-order Bézier arc emitter.                    */
  /*                                                                       */
  /*    shift    :: The shift that is applied to coordinates before they   */
  /*                are sent to the emitter.                               */
  /*                                                                       */
  /*    delta    :: The delta that is applied to coordinates before they   */
  /*                are sent to the emitter, but after the shift.          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The point coordinates sent to the emitters are the transformed     */
  /*    version of the original coordinates (this is important for high    */
  /*    accuracy during scan-conversion).  The transformation is simple:   */
  /*                                                                       */
  /*    {                                                                  */
  /*      x' = (x << shift) - delta                                        */
  /*      y' = (x << shift) - delta                                        */
  /*    }                                                                  */
  /*                                                                       */
  /*    Set the values of `shift' and `delta' to~0 to get the original     */
  /*    point coordinates.                                                 */
  /*                                                                       */
  typedef struct  FT_Outline_Funcs_
  {
	FT_Outline_MoveToFunc   move_to;
	FT_Outline_LineToFunc   line_to;
	FT_Outline_ConicToFunc  conic_to;
	FT_Outline_CubicToFunc  cubic_to;

	int                     shift;
	FT_Pos                  delta;

  } FT_Outline_Funcs;

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    basic_types                                                        */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_IMAGE_TAG                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro converts four-letter tags to an unsigned long type.     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Since many 16-bit compilers don't like 32-bit enumerations, you    */
  /*    should redefine this macro in case of problems to something like   */
  /*    this:                                                              */
  /*                                                                       */
  /*    {                                                                  */
  /*      #define FT_IMAGE_TAG( value, _x1, _x2, _x3, _x4 )  value         */
  /*    }                                                                  */
  /*                                                                       */
  /*    to get a simple enumeration without assigning special numbers.     */
  /*                                                                       */
#ifndef FT_IMAGE_TAG
#define FT_IMAGE_TAG( value, _x1, _x2, _x3, _x4 )  \
		  value = ( ( (unsigned long)_x1 << 24 ) | \
					( (unsigned long)_x2 << 16 ) | \
					( (unsigned long)_x3 << 8  ) | \
					  (unsigned long)_x4         )
#endif /* FT_IMAGE_TAG */

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Glyph_Format                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration type used to describe the format of a given glyph   */
  /*    image.  Note that this version of FreeType only supports two image */
  /*    formats, even though future font drivers will be able to register  */
  /*    their own format.                                                  */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_GLYPH_FORMAT_NONE ::                                            */
  /*      The value~0 is reserved.                                         */
  /*                                                                       */
  /*    FT_GLYPH_FORMAT_COMPOSITE ::                                       */
  /*      The glyph image is a composite of several other images.  This    */
  /*      format is _only_ used with @FT_LOAD_NO_RECURSE, and is used to   */
  /*      report compound glyphs (like accented characters).               */
  /*                                                                       */
  /*    FT_GLYPH_FORMAT_BITMAP ::                                          */
  /*      The glyph image is a bitmap, and can be described as an          */
  /*      @FT_Bitmap.  You generally need to access the `bitmap' field of  */
  /*      the @FT_GlyphSlotRec structure to read it.                       */
  /*                                                                       */
  /*    FT_GLYPH_FORMAT_OUTLINE ::                                         */
  /*      The glyph image is a vectorial outline made of line segments     */
  /*      and Bézier arcs; it can be described as an @FT_Outline; you      */
  /*      generally want to access the `outline' field of the              */
  /*      @FT_GlyphSlotRec structure to read it.                           */
  /*                                                                       */
  /*    FT_GLYPH_FORMAT_PLOTTER ::                                         */
  /*      The glyph image is a vectorial path with no inside and outside   */
  /*      contours.  Some Type~1 fonts, like those in the Hershey family,  */
  /*      contain glyphs in this format.  These are described as           */
  /*      @FT_Outline, but FreeType isn't currently capable of rendering   */
  /*      them correctly.                                                  */
  /*                                                                       */
  typedef enum  FT_Glyph_Format_
  {
	FT_IMAGE_TAG( FT_GLYPH_FORMAT_NONE, 0, 0, 0, 0 ),

	FT_IMAGE_TAG( FT_GLYPH_FORMAT_COMPOSITE, 'c', 'o', 'm', 'p' ),
	FT_IMAGE_TAG( FT_GLYPH_FORMAT_BITMAP,    'b', 'i', 't', 's' ),
	FT_IMAGE_TAG( FT_GLYPH_FORMAT_OUTLINE,   'o', 'u', 't', 'l' ),
	FT_IMAGE_TAG( FT_GLYPH_FORMAT_PLOTTER,   'p', 'l', 'o', 't' )

  } FT_Glyph_Format;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    ft_glyph_format_xxx                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A list of deprecated constants.  Use the corresponding             */
  /*    @FT_Glyph_Format values instead.                                   */
  /*                                                                       */
  /* <Values>                                                              */
  /*    ft_glyph_format_none      :: See @FT_GLYPH_FORMAT_NONE.            */
  /*    ft_glyph_format_composite :: See @FT_GLYPH_FORMAT_COMPOSITE.       */
  /*    ft_glyph_format_bitmap    :: See @FT_GLYPH_FORMAT_BITMAP.          */
  /*    ft_glyph_format_outline   :: See @FT_GLYPH_FORMAT_OUTLINE.         */
  /*    ft_glyph_format_plotter   :: See @FT_GLYPH_FORMAT_PLOTTER.         */
  /*                                                                       */
#define ft_glyph_format_none       FT_GLYPH_FORMAT_NONE
#define ft_glyph_format_composite  FT_GLYPH_FORMAT_COMPOSITE
#define ft_glyph_format_bitmap     FT_GLYPH_FORMAT_BITMAP
#define ft_glyph_format_outline    FT_GLYPH_FORMAT_OUTLINE
#define ft_glyph_format_plotter    FT_GLYPH_FORMAT_PLOTTER

  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****            R A S T E R   D E F I N I T I O N S                *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* A raster is a scan converter, in charge of rendering an outline into  */
  /* a a bitmap.  This section contains the public API for rasters.        */
  /*                                                                       */
  /* Note that in FreeType 2, all rasters are now encapsulated within      */
  /* specific modules called `renderers'.  See `freetype/ftrender.h' for   */
  /* more details on renderers.                                            */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    raster                                                             */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Scanline Converter                                                 */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    How vectorial outlines are converted into bitmaps and pixmaps.     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains technical definitions.                       */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Raster                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle (pointer) to a raster object.  Each object can be used    */
  /*    independently to convert an outline into a bitmap or pixmap.       */
  /*                                                                       */
  typedef struct FT_RasterRec_*  FT_Raster;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Span                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model a single span of gray (or black) pixels  */
  /*    when rendering a monochrome or anti-aliased bitmap.                */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x        :: The span's horizontal start position.                  */
  /*                                                                       */
  /*    len      :: The span's length in pixels.                           */
  /*                                                                       */
  /*    coverage :: The span color/coverage, ranging from 0 (background)   */
  /*                to 255 (foreground).  Only used for anti-aliased       */
  /*                rendering.                                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This structure is used by the span drawing callback type named     */
  /*    @FT_SpanFunc which takes the y~coordinate of the span as a         */
  /*    a parameter.                                                       */
  /*                                                                       */
  /*    The coverage value is always between 0 and 255.  If you want less  */
  /*    gray values, the callback function has to reduce them.             */
  /*                                                                       */
  typedef struct  FT_Span_
  {
	short           x;
	unsigned short  len;
	unsigned char   coverage;

  } FT_Span;

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_SpanFunc                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used as a call-back by the anti-aliased renderer in     */
  /*    order to let client applications draw themselves the gray pixel    */
  /*    spans on each scan line.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y     :: The scanline's y~coordinate.                              */
  /*                                                                       */
  /*    count :: The number of spans to draw on this scanline.             */
  /*                                                                       */
  /*    spans :: A table of `count' spans to draw on the scanline.         */
  /*                                                                       */
  /*    user  :: User-supplied data that is passed to the callback.        */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This callback allows client applications to directly render the    */
  /*    gray spans of the anti-aliased bitmap to any kind of surfaces.     */
  /*                                                                       */
  /*    This can be used to write anti-aliased outlines directly to a      */
  /*    given background bitmap, and even perform translucency.            */
  /*                                                                       */
  /*    Note that the `count' field cannot be greater than a fixed value   */
  /*    defined by the `FT_MAX_GRAY_SPANS' configuration macro in          */
  /*    `ftoption.h'.  By default, this value is set to~32, which means    */
  /*    that if there are more than 32~spans on a given scanline, the      */
  /*    callback is called several times with the same `y' parameter in    */
  /*    order to draw all callbacks.                                       */
  /*                                                                       */
  /*    Otherwise, the callback is only called once per scan-line, and     */
  /*    only for those scanlines that do have `gray' pixels on them.       */
  /*                                                                       */
  typedef void
  (*FT_SpanFunc)( int             y,
				  int             count,
				  const FT_Span*  spans,
				  void*           user );

#define FT_Raster_Span_Func  FT_SpanFunc

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_BitTest_Func                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    THIS TYPE IS DEPRECATED.  DO NOT USE IT.                           */
  /*                                                                       */
  /*    A function used as a call-back by the monochrome scan-converter    */
  /*    to test whether a given target pixel is already set to the drawing */
  /*    `color'.  These tests are crucial to implement drop-out control    */
  /*    per-se the TrueType spec.                                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y     :: The pixel's y~coordinate.                                 */
  /*                                                                       */
  /*    x     :: The pixel's x~coordinate.                                 */
  /*                                                                       */
  /*    user  :: User-supplied data that is passed to the callback.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*   1~if the pixel is `set', 0~otherwise.                               */
  /*                                                                       */
  typedef int
  (*FT_Raster_BitTest_Func)( int    y,
							 int    x,
							 void*  user );

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_BitSet_Func                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    THIS TYPE IS DEPRECATED.  DO NOT USE IT.                           */
  /*                                                                       */
  /*    A function used as a call-back by the monochrome scan-converter    */
  /*    to set an individual target pixel.  This is crucial to implement   */
  /*    drop-out control according to the TrueType specification.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    y     :: The pixel's y~coordinate.                                 */
  /*                                                                       */
  /*    x     :: The pixel's x~coordinate.                                 */
  /*                                                                       */
  /*    user  :: User-supplied data that is passed to the callback.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    1~if the pixel is `set', 0~otherwise.                              */
  /*                                                                       */
  typedef void
  (*FT_Raster_BitSet_Func)( int    y,
							int    x,
							void*  user );

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_RASTER_FLAG_XXX                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A list of bit flag constants as used in the `flags' field of a     */
  /*    @FT_Raster_Params structure.                                       */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_RASTER_FLAG_DEFAULT :: This value is 0.                         */
  /*                                                                       */
  /*    FT_RASTER_FLAG_AA      :: This flag is set to indicate that an     */
  /*                              anti-aliased glyph image should be       */
  /*                              generated.  Otherwise, it will be        */
  /*                              monochrome (1-bit).                      */
  /*                                                                       */
  /*    FT_RASTER_FLAG_DIRECT  :: This flag is set to indicate direct      */
  /*                              rendering.  In this mode, client         */
  /*                              applications must provide their own span */
  /*                              callback.  This lets them directly       */
  /*                              draw or compose over an existing bitmap. */
  /*                              If this bit is not set, the target       */
  /*                              pixmap's buffer _must_ be zeroed before  */
  /*                              rendering.                               */
  /*                                                                       */
  /*                              Note that for now, direct rendering is   */
  /*                              only possible with anti-aliased glyphs.  */
  /*                                                                       */
  /*    FT_RASTER_FLAG_CLIP    :: This flag is only used in direct         */
  /*                              rendering mode.  If set, the output will */
  /*                              be clipped to a box specified in the     */
  /*                              `clip_box' field of the                  */
  /*                              @FT_Raster_Params structure.             */
  /*                                                                       */
  /*                              Note that by default, the glyph bitmap   */
  /*                              is clipped to the target pixmap, except  */
  /*                              in direct rendering mode where all spans */
  /*                              are generated if no clipping box is set. */
  /*                                                                       */
#define FT_RASTER_FLAG_DEFAULT  0x0
#define FT_RASTER_FLAG_AA       0x1
#define FT_RASTER_FLAG_DIRECT   0x2
#define FT_RASTER_FLAG_CLIP     0x4

  /* deprecated */
#define ft_raster_flag_default  FT_RASTER_FLAG_DEFAULT
#define ft_raster_flag_aa       FT_RASTER_FLAG_AA
#define ft_raster_flag_direct   FT_RASTER_FLAG_DIRECT
#define ft_raster_flag_clip     FT_RASTER_FLAG_CLIP

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Raster_Params                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure to hold the arguments used by a raster's render        */
  /*    function.                                                          */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    target      :: The target bitmap.                                  */
  /*                                                                       */
  /*    source      :: A pointer to the source glyph image (e.g., an       */
  /*                   @FT_Outline).                                       */
  /*                                                                       */
  /*    flags       :: The rendering flags.                                */
  /*                                                                       */
  /*    gray_spans  :: The gray span drawing callback.                     */
  /*                                                                       */
  /*    black_spans :: The black span drawing callback.  UNIMPLEMENTED!    */
  /*                                                                       */
  /*    bit_test    :: The bit test callback.  UNIMPLEMENTED!              */
  /*                                                                       */
  /*    bit_set     :: The bit set callback.  UNIMPLEMENTED!               */
  /*                                                                       */
  /*    user        :: User-supplied data that is passed to each drawing   */
  /*                   callback.                                           */
  /*                                                                       */
  /*    clip_box    :: An optional clipping box.  It is only used in       */
  /*                   direct rendering mode.  Note that coordinates here  */
  /*                   should be expressed in _integer_ pixels (and not in */
  /*                   26.6 fixed-point units).                            */
  /*                                                                       */
  /* <Note>                                                                */
  /*    An anti-aliased glyph bitmap is drawn if the @FT_RASTER_FLAG_AA    */
  /*    bit flag is set in the `flags' field, otherwise a monochrome       */
  /*    bitmap is generated.                                               */
  /*                                                                       */
  /*    If the @FT_RASTER_FLAG_DIRECT bit flag is set in `flags', the      */
  /*    raster will call the `gray_spans' callback to draw gray pixel      */
  /*    spans, in the case of an aa glyph bitmap, it will call             */
  /*    `black_spans', and `bit_test' and `bit_set' in the case of a       */
  /*    monochrome bitmap.  This allows direct composition over a          */
  /*    pre-existing bitmap through user-provided callbacks to perform the */
  /*    span drawing/composition.                                          */
  /*                                                                       */
  /*    Note that the `bit_test' and `bit_set' callbacks are required when */
  /*    rendering a monochrome bitmap, as they are crucial to implement    */
  /*    correct drop-out control as defined in the TrueType specification. */
  /*                                                                       */
  typedef struct  FT_Raster_Params_
  {
	const FT_Bitmap*        target;
	const void*             source;
	int                     flags;
	FT_SpanFunc             gray_spans;
	FT_SpanFunc             black_spans;  /* doesn't work! */
	FT_Raster_BitTest_Func  bit_test;     /* doesn't work! */
	FT_Raster_BitSet_Func   bit_set;      /* doesn't work! */
	void*                   user;
	FT_BBox                 clip_box;

  } FT_Raster_Params;

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_NewFunc                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to create a new raster object.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    memory :: A handle to the memory allocator.                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    raster :: A handle to the new raster object.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0~means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The `memory' parameter is a typeless pointer in order to avoid     */
  /*    un-wanted dependencies on the rest of the FreeType code.  In       */
  /*    practice, it is an @FT_Memory object, i.e., a handle to the        */
  /*    standard FreeType memory allocator.  However, this field can be    */
  /*    completely ignored by a given raster implementation.               */
  /*                                                                       */
  typedef int
  (*FT_Raster_NewFunc)( void*       memory,
						FT_Raster*  raster );

#define FT_Raster_New_Func  FT_Raster_NewFunc

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_DoneFunc                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to destroy a given raster object.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    raster :: A handle to the raster object.                           */
  /*                                                                       */
  typedef void
  (*FT_Raster_DoneFunc)( FT_Raster  raster );

#define FT_Raster_Done_Func  FT_Raster_DoneFunc

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_ResetFunc                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    FreeType provides an area of memory called the `render pool',      */
  /*    available to all registered rasters.  This pool can be freely used */
  /*    during a given scan-conversion but is shared by all rasters.  Its  */
  /*    content is thus transient.                                         */
  /*                                                                       */
  /*    This function is called each time the render pool changes, or just */
  /*    after a new raster object is created.                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    raster    :: A handle to the new raster object.                    */
  /*                                                                       */
  /*    pool_base :: The address in memory of the render pool.             */
  /*                                                                       */
  /*    pool_size :: The size in bytes of the render pool.                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Rasters can ignore the render pool and rely on dynamic memory      */
  /*    allocation if they want to (a handle to the memory allocator is    */
  /*    passed to the raster constructor).  However, this is not           */
  /*    recommended for efficiency purposes.                               */
  /*                                                                       */
  typedef void
  (*FT_Raster_ResetFunc)( FT_Raster       raster,
						  unsigned char*  pool_base,
						  unsigned long   pool_size );

#define FT_Raster_Reset_Func  FT_Raster_ResetFunc

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_SetModeFunc                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is a generic facility to change modes or attributes  */
  /*    in a given raster.  This can be used for debugging purposes, or    */
  /*    simply to allow implementation-specific `features' in a given      */
  /*    raster module.                                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    raster :: A handle to the new raster object.                       */
  /*                                                                       */
  /*    mode   :: A 4-byte tag used to name the mode or property.          */
  /*                                                                       */
  /*    args   :: A pointer to the new mode/property to use.               */
  /*                                                                       */
  typedef int
  (*FT_Raster_SetModeFunc)( FT_Raster      raster,
							unsigned long  mode,
							void*          args );

#define FT_Raster_Set_Mode_Func  FT_Raster_SetModeFunc

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Raster_RenderFunc                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Invoke a given raster to scan-convert a given glyph image into a   */
  /*    target bitmap.                                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    raster :: A handle to the raster object.                           */
  /*                                                                       */
  /*    params :: A pointer to an @FT_Raster_Params structure used to      */
  /*              store the rendering parameters.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    Error code.  0~means success.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The exact format of the source image depends on the raster's glyph */
  /*    format defined in its @FT_Raster_Funcs structure.  It can be an    */
  /*    @FT_Outline or anything else in order to support a large array of  */
  /*    glyph formats.                                                     */
  /*                                                                       */
  /*    Note also that the render function can fail and return a           */
  /*    `FT_Err_Unimplemented_Feature' error code if the raster used does  */
  /*    not support direct composition.                                    */
  /*                                                                       */
  /*    XXX: For now, the standard raster doesn't support direct           */
  /*         composition but this should change for the final release (see */
  /*         the files `demos/src/ftgrays.c' and `demos/src/ftgrays2.c'    */
  /*         for examples of distinct implementations which support direct */
  /*         composition).                                                 */
  /*                                                                       */
  typedef int
  (*FT_Raster_RenderFunc)( FT_Raster                raster,
						   const FT_Raster_Params*  params );

#define FT_Raster_Render_Func  FT_Raster_RenderFunc

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Raster_Funcs                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*   A structure used to describe a given raster class to the library.   */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    glyph_format  :: The supported glyph format for this raster.       */
  /*                                                                       */
  /*    raster_new    :: The raster constructor.                           */
  /*                                                                       */
  /*    raster_reset  :: Used to reset the render pool within the raster.  */
  /*                                                                       */
  /*    raster_render :: A function to render a glyph into a given bitmap. */
  /*                                                                       */
  /*    raster_done   :: The raster destructor.                            */
  /*                                                                       */
  typedef struct  FT_Raster_Funcs_
  {
	FT_Glyph_Format        glyph_format;
	FT_Raster_NewFunc      raster_new;
	FT_Raster_ResetFunc    raster_reset;
	FT_Raster_SetModeFunc  raster_set_mode;
	FT_Raster_RenderFunc   raster_render;
	FT_Raster_DoneFunc     raster_done;

  } FT_Raster_Funcs;

  /* */

FT_END_HEADER

#endif /* __FTIMAGE_H__ */

/* END */

/* Local Variables: */
/* coding: utf-8    */
/* End:             */

/*** End of inlined file: ftimage.h ***/

#include <stddef.h>

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    basic_types                                                        */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Basic Data Types                                                   */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    The basic data types defined by the library.                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains the basic data types defined by FreeType~2,  */
  /*    ranging from simple scalar types to bitmap descriptors.  More      */
  /*    font-specific structures are defined in a different section.       */
  /*                                                                       */
  /* <Order>                                                               */
  /*    FT_Byte                                                            */
  /*    FT_Bytes                                                           */
  /*    FT_Char                                                            */
  /*    FT_Int                                                             */
  /*    FT_UInt                                                            */
  /*    FT_Int16                                                           */
  /*    FT_UInt16                                                          */
  /*    FT_Int32                                                           */
  /*    FT_UInt32                                                          */
  /*    FT_Short                                                           */
  /*    FT_UShort                                                          */
  /*    FT_Long                                                            */
  /*    FT_ULong                                                           */
  /*    FT_Bool                                                            */
  /*    FT_Offset                                                          */
  /*    FT_PtrDist                                                         */
  /*    FT_String                                                          */
  /*    FT_Tag                                                             */
  /*    FT_Error                                                           */
  /*    FT_Fixed                                                           */
  /*    FT_Pointer                                                         */
  /*    FT_Pos                                                             */
  /*    FT_Vector                                                          */
  /*    FT_BBox                                                            */
  /*    FT_Matrix                                                          */
  /*    FT_FWord                                                           */
  /*    FT_UFWord                                                          */
  /*    FT_F2Dot14                                                         */
  /*    FT_UnitVector                                                      */
  /*    FT_F26Dot6                                                         */
  /*                                                                       */
  /*                                                                       */
  /*    FT_Generic                                                         */
  /*    FT_Generic_Finalizer                                               */
  /*                                                                       */
  /*    FT_Bitmap                                                          */
  /*    FT_Pixel_Mode                                                      */
  /*    FT_Palette_Mode                                                    */
  /*    FT_Glyph_Format                                                    */
  /*    FT_IMAGE_TAG                                                       */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Bool                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef of unsigned char, used for simple booleans.  As usual,   */
  /*    values 1 and~0 represent true and false, respectively.             */
  /*                                                                       */
  typedef unsigned char  FT_Bool;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_FWord                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 16-bit integer used to store a distance in original font  */
  /*    units.                                                             */
  /*                                                                       */
  typedef signed short  FT_FWord;   /* distance in FUnits */

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UFWord                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An unsigned 16-bit integer used to store a distance in original    */
  /*    font units.                                                        */
  /*                                                                       */
  typedef unsigned short  FT_UFWord;  /* unsigned distance */

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Char                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the _signed_ char type.                       */
  /*                                                                       */
  typedef signed char  FT_Char;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Byte                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the _unsigned_ char type.                     */
  /*                                                                       */
  typedef unsigned char  FT_Byte;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Bytes                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for constant memory areas.                               */
  /*                                                                       */
  typedef const FT_Byte*  FT_Bytes;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Tag                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for 32-bit tags (as used in the SFNT format).            */
  /*                                                                       */
  typedef FT_UInt32  FT_Tag;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_String                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the char type, usually used for strings.      */
  /*                                                                       */
  typedef char  FT_String;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Short                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for signed short.                                        */
  /*                                                                       */
  typedef signed short  FT_Short;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UShort                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for unsigned short.                                      */
  /*                                                                       */
  typedef unsigned short  FT_UShort;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Int                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for the int type.                                        */
  /*                                                                       */
  typedef signed int  FT_Int;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UInt                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for the unsigned int type.                               */
  /*                                                                       */
  typedef unsigned int  FT_UInt;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Long                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for signed long.                                         */
  /*                                                                       */
  typedef signed long  FT_Long;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_ULong                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for unsigned long.                                       */
  /*                                                                       */
  typedef unsigned long  FT_ULong;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_F2Dot14                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 2.14 fixed float type used for unit vectors.              */
  /*                                                                       */
  typedef signed short  FT_F2Dot14;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_F26Dot6                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 26.6 fixed float type used for vectorial pixel            */
  /*    coordinates.                                                       */
  /*                                                                       */
  typedef signed long  FT_F26Dot6;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Fixed                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This type is used to store 16.16 fixed float values, like scaling  */
  /*    values or matrix coefficients.                                     */
  /*                                                                       */
  typedef signed long  FT_Fixed;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Error                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The FreeType error code type.  A value of~0 is always interpreted  */
  /*    as a successful operation.                                         */
  /*                                                                       */
  typedef int  FT_Error;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Pointer                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for a typeless pointer.                           */
  /*                                                                       */
  typedef void*  FT_Pointer;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Offset                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This is equivalent to the ANSI~C `size_t' type, i.e., the largest  */
  /*    _unsigned_ integer type used to express a file size or position,   */
  /*    or a memory block size.                                            */
  /*                                                                       */
  typedef size_t  FT_Offset;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_PtrDist                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This is equivalent to the ANSI~C `ptrdiff_t' type, i.e., the       */
  /*    largest _signed_ integer type used to express the distance         */
  /*    between two pointers.                                              */
  /*                                                                       */
  typedef ft_ptrdiff_t  FT_PtrDist;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_UnitVector                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2D vector unit vector.  Uses    */
  /*    FT_F2Dot14 types.                                                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x :: Horizontal coordinate.                                        */
  /*                                                                       */
  /*    y :: Vertical coordinate.                                          */
  /*                                                                       */
  typedef struct  FT_UnitVector_
  {
	FT_F2Dot14  x;
	FT_F2Dot14  y;

  } FT_UnitVector;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Matrix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2x2 matrix.  Coefficients are   */
  /*    in 16.16 fixed float format.  The computation performed is:        */
  /*                                                                       */
  /*       {                                                               */
  /*          x' = x*xx + y*xy                                             */
  /*          y' = x*yx + y*yy                                             */
  /*       }                                                               */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    xx :: Matrix coefficient.                                          */
  /*                                                                       */
  /*    xy :: Matrix coefficient.                                          */
  /*                                                                       */
  /*    yx :: Matrix coefficient.                                          */
  /*                                                                       */
  /*    yy :: Matrix coefficient.                                          */
  /*                                                                       */
  typedef struct  FT_Matrix_
  {
	FT_Fixed  xx, xy;
	FT_Fixed  yx, yy;

  } FT_Matrix;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Data                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Read-only binary data represented as a pointer and a length.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    pointer :: The data.                                               */
  /*                                                                       */
  /*    length  :: The length of the data in bytes.                        */
  /*                                                                       */
  typedef struct  FT_Data_
  {
	const FT_Byte*  pointer;
	FT_Int          length;

  } FT_Data;

  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Generic_Finalizer                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Describe a function used to destroy the `client' data of any       */
  /*    FreeType object.  See the description of the @FT_Generic type for  */
  /*    details of usage.                                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    The address of the FreeType object which is under finalization.    */
  /*    Its client data is accessed through its `generic' field.           */
  /*                                                                       */
  typedef void  (*FT_Generic_Finalizer)(void*  object);

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Generic                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Client applications often need to associate their own data to a    */
  /*    variety of FreeType core objects.  For example, a text layout API  */
  /*    might want to associate a glyph cache to a given size object.      */
  /*                                                                       */
  /*    Some FreeType object contains a `generic' field, of type           */
  /*    FT_Generic, which usage is left to client applications and font    */
  /*    servers.                                                           */
  /*                                                                       */
  /*    It can be used to store a pointer to client-specific data, as well */
  /*    as the address of a `finalizer' function, which will be called by  */
  /*    FreeType when the object is destroyed (for example, the previous   */
  /*    client example would put the address of the glyph cache destructor */
  /*    in the `finalizer' field).                                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    data      :: A typeless pointer to any client-specified data. This */
  /*                 field is completely ignored by the FreeType library.  */
  /*                                                                       */
  /*    finalizer :: A pointer to a `generic finalizer' function, which    */
  /*                 will be called when the object is destroyed.  If this */
  /*                 field is set to NULL, no code will be called.         */
  /*                                                                       */
  typedef struct  FT_Generic_
  {
	void*                 data;
	FT_Generic_Finalizer  finalizer;

  } FT_Generic;

  /*************************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_MAKE_TAG                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro converts four-letter tags which are used to label       */
  /*    TrueType tables into an unsigned long to be used within FreeType.  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The produced values *must* be 32-bit integers.  Don't redefine     */
  /*    this macro.                                                        */
  /*                                                                       */
#define FT_MAKE_TAG( _x1, _x2, _x3, _x4 ) \
		  (FT_Tag)                        \
		  ( ( (FT_ULong)_x1 << 24 ) |     \
			( (FT_ULong)_x2 << 16 ) |     \
			( (FT_ULong)_x3 <<  8 ) |     \
			  (FT_ULong)_x4         )

  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                    L I S T   M A N A G E M E N T                      */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    list_processing                                                    */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_ListNode                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*     Many elements and objects in FreeType are listed through an       */
  /*     @FT_List record (see @FT_ListRec).  As its name suggests, an      */
  /*     FT_ListNode is a handle to a single list element.                 */
  /*                                                                       */
  typedef struct FT_ListNodeRec_*  FT_ListNode;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_List                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a list record (see @FT_ListRec).                       */
  /*                                                                       */
  typedef struct FT_ListRec_*  FT_List;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_ListNodeRec                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold a single list element.                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    prev :: The previous element in the list.  NULL if first.          */
  /*                                                                       */
  /*    next :: The next element in the list.  NULL if last.               */
  /*                                                                       */
  /*    data :: A typeless pointer to the listed object.                   */
  /*                                                                       */
  typedef struct  FT_ListNodeRec_
  {
	FT_ListNode  prev;
	FT_ListNode  next;
	void*        data;

  } FT_ListNodeRec;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_ListRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold a simple doubly-linked list.  These are   */
  /*    used in many parts of FreeType.                                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    head :: The head (first element) of doubly-linked list.            */
  /*                                                                       */
  /*    tail :: The tail (last element) of doubly-linked list.             */
  /*                                                                       */
  typedef struct  FT_ListRec_
  {
	FT_ListNode  head;
	FT_ListNode  tail;

  } FT_ListRec;

  /* */

#define FT_IS_EMPTY( list )  ( (list).head == 0 )

  /* return base error code (without module-specific prefix) */
#define FT_ERROR_BASE( x )    ( (x) & 0xFF )

  /* return module error code */
#define FT_ERROR_MODULE( x )  ( (x) & 0xFF00U )

#define FT_BOOL( x )  ( (FT_Bool)( x ) )

FT_END_HEADER

#endif /* __FTTYPES_H__ */

/* END */

/*** End of inlined file: fttypes.h ***/

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    user_allocation                                                    */
  /*                                                                       */
  /* <Title>                                                               */
  /*    User allocation                                                    */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    How client applications should allocate FreeType data structures.  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    FreeType assumes that structures allocated by the user and passed  */
  /*    as arguments are zeroed out except for the actual data.  In other  */
  /*    words, it is recommended to use `calloc' (or variants of it)       */
  /*    instead of `malloc' for allocation.                                */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                        B A S I C   T Y P E S                          */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    base_interface                                                     */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Base Interface                                                     */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    The FreeType~2 base font interface.                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section describes the public high-level API of FreeType~2.    */
  /*                                                                       */
  /* <Order>                                                               */
  /*    FT_Library                                                         */
  /*    FT_Face                                                            */
  /*    FT_Size                                                            */
  /*    FT_GlyphSlot                                                       */
  /*    FT_CharMap                                                         */
  /*    FT_Encoding                                                        */
  /*                                                                       */
  /*    FT_FaceRec                                                         */
  /*                                                                       */
  /*    FT_FACE_FLAG_SCALABLE                                              */
  /*    FT_FACE_FLAG_FIXED_SIZES                                           */
  /*    FT_FACE_FLAG_FIXED_WIDTH                                           */
  /*    FT_FACE_FLAG_HORIZONTAL                                            */
  /*    FT_FACE_FLAG_VERTICAL                                              */
  /*    FT_FACE_FLAG_SFNT                                                  */
  /*    FT_FACE_FLAG_KERNING                                               */
  /*    FT_FACE_FLAG_MULTIPLE_MASTERS                                      */
  /*    FT_FACE_FLAG_GLYPH_NAMES                                           */
  /*    FT_FACE_FLAG_EXTERNAL_STREAM                                       */
  /*    FT_FACE_FLAG_FAST_GLYPHS                                           */
  /*    FT_FACE_FLAG_HINTER                                                */
  /*                                                                       */
  /*    FT_STYLE_FLAG_BOLD                                                 */
  /*    FT_STYLE_FLAG_ITALIC                                               */
  /*                                                                       */
  /*    FT_SizeRec                                                         */
  /*    FT_Size_Metrics                                                    */
  /*                                                                       */
  /*    FT_GlyphSlotRec                                                    */
  /*    FT_Glyph_Metrics                                                   */
  /*    FT_SubGlyph                                                        */
  /*                                                                       */
  /*    FT_Bitmap_Size                                                     */
  /*                                                                       */
  /*    FT_Init_FreeType                                                   */
  /*    FT_Done_FreeType                                                   */
  /*                                                                       */
  /*    FT_New_Face                                                        */
  /*    FT_Done_Face                                                       */
  /*    FT_New_Memory_Face                                                 */
  /*    FT_Open_Face                                                       */
  /*    FT_Open_Args                                                       */
  /*    FT_Parameter                                                       */
  /*    FT_Attach_File                                                     */
  /*    FT_Attach_Stream                                                   */
  /*                                                                       */
  /*    FT_Set_Char_Size                                                   */
  /*    FT_Set_Pixel_Sizes                                                 */
  /*    FT_Request_Size                                                    */
  /*    FT_Select_Size                                                     */
  /*    FT_Size_Request_Type                                               */
  /*    FT_Size_Request                                                    */
  /*    FT_Set_Transform                                                   */
  /*    FT_Load_Glyph                                                      */
  /*    FT_Get_Char_Index                                                  */
  /*    FT_Get_Name_Index                                                  */
  /*    FT_Load_Char                                                       */
  /*                                                                       */
  /*    FT_OPEN_MEMORY                                                     */
  /*    FT_OPEN_STREAM                                                     */
  /*    FT_OPEN_PATHNAME                                                   */
  /*    FT_OPEN_DRIVER                                                     */
  /*    FT_OPEN_PARAMS                                                     */
  /*                                                                       */
  /*    FT_LOAD_DEFAULT                                                    */
  /*    FT_LOAD_RENDER                                                     */
  /*    FT_LOAD_MONOCHROME                                                 */
  /*    FT_LOAD_LINEAR_DESIGN                                              */
  /*    FT_LOAD_NO_SCALE                                                   */
  /*    FT_LOAD_NO_HINTING                                                 */
  /*    FT_LOAD_NO_BITMAP                                                  */
  /*    FT_LOAD_CROP_BITMAP                                                */
  /*                                                                       */
  /*    FT_LOAD_VERTICAL_LAYOUT                                            */
  /*    FT_LOAD_IGNORE_TRANSFORM                                           */
  /*    FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH                                */
  /*    FT_LOAD_FORCE_AUTOHINT                                             */
  /*    FT_LOAD_NO_RECURSE                                                 */
  /*    FT_LOAD_PEDANTIC                                                   */
  /*                                                                       */
  /*    FT_LOAD_TARGET_NORMAL                                              */
  /*    FT_LOAD_TARGET_LIGHT                                               */
  /*    FT_LOAD_TARGET_MONO                                                */
  /*    FT_LOAD_TARGET_LCD                                                 */
  /*    FT_LOAD_TARGET_LCD_V                                               */
  /*                                                                       */
  /*    FT_Render_Glyph                                                    */
  /*    FT_Render_Mode                                                     */
  /*    FT_Get_Kerning                                                     */
  /*    FT_Kerning_Mode                                                    */
  /*    FT_Get_Track_Kerning                                               */
  /*    FT_Get_Glyph_Name                                                  */
  /*    FT_Get_Postscript_Name                                             */
  /*                                                                       */
  /*    FT_CharMapRec                                                      */
  /*    FT_Select_Charmap                                                  */
  /*    FT_Set_Charmap                                                     */
  /*    FT_Get_Charmap_Index                                               */
  /*                                                                       */
  /*    FT_FSTYPE_INSTALLABLE_EMBEDDING                                    */
  /*    FT_FSTYPE_RESTRICTED_LICENSE_EMBEDDING                             */
  /*    FT_FSTYPE_PREVIEW_AND_PRINT_EMBEDDING                              */
  /*    FT_FSTYPE_EDITABLE_EMBEDDING                                       */
  /*    FT_FSTYPE_NO_SUBSETTING                                            */
  /*    FT_FSTYPE_BITMAP_EMBEDDING_ONLY                                    */
  /*                                                                       */
  /*    FT_Get_FSType_Flags                                                */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Glyph_Metrics                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model the metrics of a single glyph.  The      */
  /*    values are expressed in 26.6 fractional pixel format; if the flag  */
  /*    @FT_LOAD_NO_SCALE has been used while loading the glyph, values    */
  /*    are expressed in font units instead.                               */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    width ::                                                           */
  /*      The glyph's width.                                               */
  /*                                                                       */
  /*    height ::                                                          */
  /*      The glyph's height.                                              */
  /*                                                                       */
  /*    horiBearingX ::                                                    */
  /*      Left side bearing for horizontal layout.                         */
  /*                                                                       */
  /*    horiBearingY ::                                                    */
  /*      Top side bearing for horizontal layout.                          */
  /*                                                                       */
  /*    horiAdvance ::                                                     */
  /*      Advance width for horizontal layout.                             */
  /*                                                                       */
  /*    vertBearingX ::                                                    */
  /*      Left side bearing for vertical layout.                           */
  /*                                                                       */
  /*    vertBearingY ::                                                    */
  /*      Top side bearing for vertical layout.  Larger positive values    */
  /*      mean further below the vertical glyph origin.                    */
  /*                                                                       */
  /*    vertAdvance ::                                                     */
  /*      Advance height for vertical layout.  Positive values mean the    */
  /*      glyph has a positive advance downward.                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If not disabled with @FT_LOAD_NO_HINTING, the values represent     */
  /*    dimensions of the hinted glyph (in case hinting is applicable).    */
  /*                                                                       */
  /*    Stroking a glyph with an outside border does not increase          */
  /*    `horiAdvance' or `vertAdvance'; you have to manually adjust these  */
  /*    values to account for the added width and height.                  */
  /*                                                                       */
  typedef struct  FT_Glyph_Metrics_
  {
	FT_Pos  width;
	FT_Pos  height;

	FT_Pos  horiBearingX;
	FT_Pos  horiBearingY;
	FT_Pos  horiAdvance;

	FT_Pos  vertBearingX;
	FT_Pos  vertBearingY;
	FT_Pos  vertAdvance;

  } FT_Glyph_Metrics;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Bitmap_Size                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This structure models the metrics of a bitmap strike (i.e., a set  */
  /*    of glyphs for a given point size and resolution) in a bitmap font. */
  /*    It is used for the `available_sizes' field of @FT_Face.            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    height :: The vertical distance, in pixels, between two            */
  /*              consecutive baselines.  It is always positive.           */
  /*                                                                       */
  /*    width  :: The average width, in pixels, of all glyphs in the       */
  /*              strike.                                                  */
  /*                                                                       */
  /*    size   :: The nominal size of the strike in 26.6 fractional        */
  /*              points.  This field is not very useful.                  */
  /*                                                                       */
  /*    x_ppem :: The horizontal ppem (nominal width) in 26.6 fractional   */
  /*              pixels.                                                  */
  /*                                                                       */
  /*    y_ppem :: The vertical ppem (nominal height) in 26.6 fractional    */
  /*              pixels.                                                  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Windows FNT:                                                       */
  /*      The nominal size given in a FNT font is not reliable.  Thus when */
  /*      the driver finds it incorrect, it sets `size' to some calculated */
  /*      values and sets `x_ppem' and `y_ppem' to the pixel width and     */
  /*      height given in the font, respectively.                          */
  /*                                                                       */
  /*    TrueType embedded bitmaps:                                         */
  /*      `size', `width', and `height' values are not contained in the    */
  /*      bitmap strike itself.  They are computed from the global font    */
  /*      parameters.                                                      */
  /*                                                                       */
  typedef struct  FT_Bitmap_Size_
  {
	FT_Short  height;
	FT_Short  width;

	FT_Pos    size;

	FT_Pos    x_ppem;
	FT_Pos    y_ppem;

  } FT_Bitmap_Size;

  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                     O B J E C T   C L A S S E S                       */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Library                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a FreeType library instance.  Each `library' is        */
  /*    completely independent from the others; it is the `root' of a set  */
  /*    of objects like fonts, faces, sizes, etc.                          */
  /*                                                                       */
  /*    It also embeds a memory manager (see @FT_Memory), as well as a     */
  /*    scan-line converter object (see @FT_Raster).                       */
  /*                                                                       */
  /*    For multi-threading applications each thread should have its own   */
  /*    FT_Library object.                                                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Library objects are normally created by @FT_Init_FreeType, and     */
  /*    destroyed with @FT_Done_FreeType.                                  */
  /*                                                                       */
  typedef struct FT_LibraryRec_  *FT_Library;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Module                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given FreeType module object.  Each module can be a  */
  /*    font driver, a renderer, or anything else that provides services   */
  /*    to the formers.                                                    */
  /*                                                                       */
  typedef struct FT_ModuleRec_*  FT_Module;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Driver                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given FreeType font driver object.  Each font driver */
  /*    is a special module capable of creating faces from font files.     */
  /*                                                                       */
  typedef struct FT_DriverRec_*  FT_Driver;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Renderer                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given FreeType renderer.  A renderer is a special    */
  /*    module in charge of converting a glyph image to a bitmap, when     */
  /*    necessary.  Each renderer supports a given glyph image format, and */
  /*    one or more target surface depths.                                 */
  /*                                                                       */
  typedef struct FT_RendererRec_*  FT_Renderer;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Face                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given typographic face object.  A face object models */
  /*    a given typeface, in a given style.                                */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Each face object also owns a single @FT_GlyphSlot object, as well  */
  /*    as one or more @FT_Size objects.                                   */
  /*                                                                       */
  /*    Use @FT_New_Face or @FT_Open_Face to create a new face object from */
  /*    a given filepathname or a custom input stream.                     */
  /*                                                                       */
  /*    Use @FT_Done_Face to destroy it (along with its slot and sizes).   */
  /*                                                                       */
  /* <Also>                                                                */
  /*    See @FT_FaceRec for the publicly accessible fields of a given face */
  /*    object.                                                            */
  /*                                                                       */
  typedef struct FT_FaceRec_*  FT_Face;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Size                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an object used to model a face scaled to a given       */
  /*    character size.                                                    */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Each @FT_Face has an _active_ @FT_Size object that is used by      */
  /*    functions like @FT_Load_Glyph to determine the scaling             */
  /*    transformation which is used to load and hint glyphs and metrics.  */
  /*                                                                       */
  /*    You can use @FT_Set_Char_Size, @FT_Set_Pixel_Sizes,                */
  /*    @FT_Request_Size or even @FT_Select_Size to change the content     */
  /*    (i.e., the scaling values) of the active @FT_Size.                 */
  /*                                                                       */
  /*    You can use @FT_New_Size to create additional size objects for a   */
  /*    given @FT_Face, but they won't be used by other functions until    */
  /*    you activate it through @FT_Activate_Size.  Only one size can be   */
  /*    activated at any given time per face.                              */
  /*                                                                       */
  /* <Also>                                                                */
  /*    See @FT_SizeRec for the publicly accessible fields of a given size */
  /*    object.                                                            */
  /*                                                                       */
  typedef struct FT_SizeRec_*  FT_Size;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_GlyphSlot                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given `glyph slot'.  A slot is a container where it  */
  /*    is possible to load any of the glyphs contained in its parent      */
  /*    face.                                                              */
  /*                                                                       */
  /*    In other words, each time you call @FT_Load_Glyph or               */
  /*    @FT_Load_Char, the slot's content is erased by the new glyph data, */
  /*    i.e., the glyph's metrics, its image (bitmap or outline), and      */
  /*    other control information.                                         */
  /*                                                                       */
  /* <Also>                                                                */
  /*    See @FT_GlyphSlotRec for the publicly accessible glyph fields.     */
  /*                                                                       */
  typedef struct FT_GlyphSlotRec_*  FT_GlyphSlot;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_CharMap                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a given character map.  A charmap is used to translate */
  /*    character codes in a given encoding into glyph indexes for its     */
  /*    parent's face.  Some font formats may provide several charmaps per */
  /*    font.                                                              */
  /*                                                                       */
  /*    Each face object owns zero or more charmaps, but only one of them  */
  /*    can be `active' and used by @FT_Get_Char_Index or @FT_Load_Char.   */
  /*                                                                       */
  /*    The list of available charmaps in a face is available through the  */
  /*    `face->num_charmaps' and `face->charmaps' fields of @FT_FaceRec.   */
  /*                                                                       */
  /*    The currently active charmap is available as `face->charmap'.      */
  /*    You should call @FT_Set_Charmap to change it.                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    When a new face is created (either through @FT_New_Face or         */
  /*    @FT_Open_Face), the library looks for a Unicode charmap within     */
  /*    the list and automatically activates it.                           */
  /*                                                                       */
  /* <Also>                                                                */
  /*    See @FT_CharMapRec for the publicly accessible fields of a given   */
  /*    character map.                                                     */
  /*                                                                       */
  typedef struct FT_CharMapRec_*  FT_CharMap;

  /*************************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_ENC_TAG                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro converts four-letter tags into an unsigned long.  It is */
  /*    used to define `encoding' identifiers (see @FT_Encoding).          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Since many 16-bit compilers don't like 32-bit enumerations, you    */
  /*    should redefine this macro in case of problems to something like   */
  /*    this:                                                              */
  /*                                                                       */
  /*    {                                                                  */
  /*      #define FT_ENC_TAG( value, a, b, c, d )  value                   */
  /*    }                                                                  */
  /*                                                                       */
  /*    to get a simple enumeration without assigning special numbers.     */
  /*                                                                       */

#ifndef FT_ENC_TAG
#define FT_ENC_TAG( value, a, b, c, d )         \
		  value = ( ( (FT_UInt32)(a) << 24 ) |  \
					( (FT_UInt32)(b) << 16 ) |  \
					( (FT_UInt32)(c) <<  8 ) |  \
					  (FT_UInt32)(d)         )

#endif /* FT_ENC_TAG */

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Encoding                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration used to specify character sets supported by         */
  /*    charmaps.  Used in the @FT_Select_Charmap API function.            */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Despite the name, this enumeration lists specific character        */
  /*    repertories (i.e., charsets), and not text encoding methods (e.g., */
  /*    UTF-8, UTF-16, etc.).                                              */
  /*                                                                       */
  /*    Other encodings might be defined in the future.                    */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_ENCODING_NONE ::                                                */
  /*      The encoding value~0 is reserved.                                */
  /*                                                                       */
  /*    FT_ENCODING_UNICODE ::                                             */
  /*      Corresponds to the Unicode character set.  This value covers     */
  /*      all versions of the Unicode repertoire, including ASCII and      */
  /*      Latin-1.  Most fonts include a Unicode charmap, but not all      */
  /*      of them.                                                         */
  /*                                                                       */
  /*      For example, if you want to access Unicode value U+1F028 (and    */
  /*      the font contains it), use value 0x1F028 as the input value for  */
  /*      @FT_Get_Char_Index.                                              */
  /*                                                                       */
  /*    FT_ENCODING_MS_SYMBOL ::                                           */
  /*      Corresponds to the Microsoft Symbol encoding, used to encode     */
  /*      mathematical symbols in the 32..255 character code range.  For   */
  /*      more information, see `http://www.ceviz.net/symbol.htm'.         */
  /*                                                                       */
  /*    FT_ENCODING_SJIS ::                                                */
  /*      Corresponds to Japanese SJIS encoding.  More info at             */
  /*      at `http://langsupport.japanreference.com/encoding.shtml'.       */
  /*      See note on multi-byte encodings below.                          */
  /*                                                                       */
  /*    FT_ENCODING_GB2312 ::                                              */
  /*      Corresponds to an encoding system for Simplified Chinese as used */
  /*      used in mainland China.                                          */
  /*                                                                       */
  /*    FT_ENCODING_BIG5 ::                                                */
  /*      Corresponds to an encoding system for Traditional Chinese as     */
  /*      used in Taiwan and Hong Kong.                                    */
  /*                                                                       */
  /*    FT_ENCODING_WANSUNG ::                                             */
  /*      Corresponds to the Korean encoding system known as Wansung.      */
  /*      For more information see                                         */
  /*      `http://www.microsoft.com/typography/unicode/949.txt'.           */
  /*                                                                       */
  /*    FT_ENCODING_JOHAB ::                                               */
  /*      The Korean standard character set (KS~C 5601-1992), which        */
  /*      corresponds to MS Windows code page 1361.  This character set    */
  /*      includes all possible Hangeul character combinations.            */
  /*                                                                       */
  /*    FT_ENCODING_ADOBE_LATIN_1 ::                                       */
  /*      Corresponds to a Latin-1 encoding as defined in a Type~1         */
  /*      PostScript font.  It is limited to 256 character codes.          */
  /*                                                                       */
  /*    FT_ENCODING_ADOBE_STANDARD ::                                      */
  /*      Corresponds to the Adobe Standard encoding, as found in Type~1,  */
  /*      CFF, and OpenType/CFF fonts.  It is limited to 256 character     */
  /*      codes.                                                           */
  /*                                                                       */
  /*    FT_ENCODING_ADOBE_EXPERT ::                                        */
  /*      Corresponds to the Adobe Expert encoding, as found in Type~1,    */
  /*      CFF, and OpenType/CFF fonts.  It is limited to 256 character     */
  /*      codes.                                                           */
  /*                                                                       */
  /*    FT_ENCODING_ADOBE_CUSTOM ::                                        */
  /*      Corresponds to a custom encoding, as found in Type~1, CFF, and   */
  /*      OpenType/CFF fonts.  It is limited to 256 character codes.       */
  /*                                                                       */
  /*    FT_ENCODING_APPLE_ROMAN ::                                         */
  /*      Corresponds to the 8-bit Apple roman encoding.  Many TrueType    */
  /*      and OpenType fonts contain a charmap for this encoding, since    */
  /*      older versions of Mac OS are able to use it.                     */
  /*                                                                       */
  /*    FT_ENCODING_OLD_LATIN_2 ::                                         */
  /*      This value is deprecated and was never used nor reported by      */
  /*      FreeType.  Don't use or test for it.                             */
  /*                                                                       */
  /*    FT_ENCODING_MS_SJIS ::                                             */
  /*      Same as FT_ENCODING_SJIS.  Deprecated.                           */
  /*                                                                       */
  /*    FT_ENCODING_MS_GB2312 ::                                           */
  /*      Same as FT_ENCODING_GB2312.  Deprecated.                         */
  /*                                                                       */
  /*    FT_ENCODING_MS_BIG5 ::                                             */
  /*      Same as FT_ENCODING_BIG5.  Deprecated.                           */
  /*                                                                       */
  /*    FT_ENCODING_MS_WANSUNG ::                                          */
  /*      Same as FT_ENCODING_WANSUNG.  Deprecated.                        */
  /*                                                                       */
  /*    FT_ENCODING_MS_JOHAB ::                                            */
  /*      Same as FT_ENCODING_JOHAB.  Deprecated.                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    By default, FreeType automatically synthesizes a Unicode charmap   */
  /*    for PostScript fonts, using their glyph names dictionaries.        */
  /*    However, it also reports the encodings defined explicitly in the   */
  /*    font file, for the cases when they are needed, with the Adobe      */
  /*    values as well.                                                    */
  /*                                                                       */
  /*    FT_ENCODING_NONE is set by the BDF and PCF drivers if the charmap  */
  /*    is neither Unicode nor ISO-8859-1 (otherwise it is set to          */
  /*    FT_ENCODING_UNICODE).  Use @FT_Get_BDF_Charset_ID to find out      */
  /*    which encoding is really present.  If, for example, the            */
  /*    `cs_registry' field is `KOI8' and the `cs_encoding' field is `R',  */
  /*    the font is encoded in KOI8-R.                                     */
  /*                                                                       */
  /*    FT_ENCODING_NONE is always set (with a single exception) by the    */
  /*    winfonts driver.  Use @FT_Get_WinFNT_Header and examine the        */
  /*    `charset' field of the @FT_WinFNT_HeaderRec structure to find out  */
  /*    which encoding is really present.  For example,                    */
  /*    @FT_WinFNT_ID_CP1251 (204) means Windows code page 1251 (for       */
  /*    Russian).                                                          */
  /*                                                                       */
  /*    FT_ENCODING_NONE is set if `platform_id' is @TT_PLATFORM_MACINTOSH */
  /*    and `encoding_id' is not @TT_MAC_ID_ROMAN (otherwise it is set to  */
  /*    FT_ENCODING_APPLE_ROMAN).                                          */
  /*                                                                       */
  /*    If `platform_id' is @TT_PLATFORM_MACINTOSH, use the function       */
  /*    @FT_Get_CMap_Language_ID  to query the Mac language ID which may   */
  /*    be needed to be able to distinguish Apple encoding variants.  See  */
  /*                                                                       */
  /*      http://www.unicode.org/Public/MAPPINGS/VENDORS/APPLE/README.TXT  */
  /*                                                                       */
  /*    to get an idea how to do that.  Basically, if the language ID      */
  /*    is~0, don't use it, otherwise subtract 1 from the language ID.     */
  /*    Then examine `encoding_id'.  If, for example, `encoding_id' is     */
  /*    @TT_MAC_ID_ROMAN and the language ID (minus~1) is                  */
  /*    `TT_MAC_LANGID_GREEK', it is the Greek encoding, not Roman.        */
  /*    @TT_MAC_ID_ARABIC with `TT_MAC_LANGID_FARSI' means the Farsi       */
  /*    variant the Arabic encoding.                                       */
  /*                                                                       */
  typedef enum  FT_Encoding_
  {
	FT_ENC_TAG( FT_ENCODING_NONE, 0, 0, 0, 0 ),

	FT_ENC_TAG( FT_ENCODING_MS_SYMBOL, 's', 'y', 'm', 'b' ),
	FT_ENC_TAG( FT_ENCODING_UNICODE,   'u', 'n', 'i', 'c' ),

	FT_ENC_TAG( FT_ENCODING_SJIS,    's', 'j', 'i', 's' ),
	FT_ENC_TAG( FT_ENCODING_GB2312,  'g', 'b', ' ', ' ' ),
	FT_ENC_TAG( FT_ENCODING_BIG5,    'b', 'i', 'g', '5' ),
	FT_ENC_TAG( FT_ENCODING_WANSUNG, 'w', 'a', 'n', 's' ),
	FT_ENC_TAG( FT_ENCODING_JOHAB,   'j', 'o', 'h', 'a' ),

	/* for backwards compatibility */
	FT_ENCODING_MS_SJIS    = FT_ENCODING_SJIS,
	FT_ENCODING_MS_GB2312  = FT_ENCODING_GB2312,
	FT_ENCODING_MS_BIG5    = FT_ENCODING_BIG5,
	FT_ENCODING_MS_WANSUNG = FT_ENCODING_WANSUNG,
	FT_ENCODING_MS_JOHAB   = FT_ENCODING_JOHAB,

	FT_ENC_TAG( FT_ENCODING_ADOBE_STANDARD, 'A', 'D', 'O', 'B' ),
	FT_ENC_TAG( FT_ENCODING_ADOBE_EXPERT,   'A', 'D', 'B', 'E' ),
	FT_ENC_TAG( FT_ENCODING_ADOBE_CUSTOM,   'A', 'D', 'B', 'C' ),
	FT_ENC_TAG( FT_ENCODING_ADOBE_LATIN_1,  'l', 'a', 't', '1' ),

	FT_ENC_TAG( FT_ENCODING_OLD_LATIN_2, 'l', 'a', 't', '2' ),

	FT_ENC_TAG( FT_ENCODING_APPLE_ROMAN, 'a', 'r', 'm', 'n' )

  } FT_Encoding;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    ft_encoding_xxx                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    These constants are deprecated; use the corresponding @FT_Encoding */
  /*    values instead.                                                    */
  /*                                                                       */
#define ft_encoding_none            FT_ENCODING_NONE
#define ft_encoding_unicode         FT_ENCODING_UNICODE
#define ft_encoding_symbol          FT_ENCODING_MS_SYMBOL
#define ft_encoding_latin_1         FT_ENCODING_ADOBE_LATIN_1
#define ft_encoding_latin_2         FT_ENCODING_OLD_LATIN_2
#define ft_encoding_sjis            FT_ENCODING_SJIS
#define ft_encoding_gb2312          FT_ENCODING_GB2312
#define ft_encoding_big5            FT_ENCODING_BIG5
#define ft_encoding_wansung         FT_ENCODING_WANSUNG
#define ft_encoding_johab           FT_ENCODING_JOHAB

#define ft_encoding_adobe_standard  FT_ENCODING_ADOBE_STANDARD
#define ft_encoding_adobe_expert    FT_ENCODING_ADOBE_EXPERT
#define ft_encoding_adobe_custom    FT_ENCODING_ADOBE_CUSTOM
#define ft_encoding_apple_roman     FT_ENCODING_APPLE_ROMAN

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_CharMapRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The base charmap structure.                                        */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    face        :: A handle to the parent face object.                 */
  /*                                                                       */
  /*    encoding    :: An @FT_Encoding tag identifying the charmap.  Use   */
  /*                   this with @FT_Select_Charmap.                       */
  /*                                                                       */
  /*    platform_id :: An ID number describing the platform for the        */
  /*                   following encoding ID.  This comes directly from    */
  /*                   the TrueType specification and should be emulated   */
  /*                   for other formats.                                  */
  /*                                                                       */
  /*    encoding_id :: A platform specific encoding number.  This also     */
  /*                   comes from the TrueType specification and should be */
  /*                   emulated similarly.                                 */
  /*                                                                       */
  typedef struct  FT_CharMapRec_
  {
	FT_Face      face;
	FT_Encoding  encoding;
	FT_UShort    platform_id;
	FT_UShort    encoding_id;

  } FT_CharMapRec;

  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                 B A S E   O B J E C T   C L A S S E S                 */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Face_Internal                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An opaque handle to an `FT_Face_InternalRec' structure, used to    */
  /*    model private data of a given @FT_Face object.                     */
  /*                                                                       */
  /*    This structure might change between releases of FreeType~2 and is  */
  /*    not generally available to client applications.                    */
  /*                                                                       */
  typedef struct FT_Face_InternalRec_*  FT_Face_Internal;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_FaceRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    FreeType root face class structure.  A face object models a        */
  /*    typeface in a font file.                                           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_faces           :: The number of faces in the font file.  Some */
  /*                           font formats can have multiple faces in     */
  /*                           a font file.                                */
  /*                                                                       */
  /*    face_index          :: The index of the face in the font file.  It */
  /*                           is set to~0 if there is only one face in    */
  /*                           the font file.                              */
  /*                                                                       */
  /*    face_flags          :: A set of bit flags that give important      */
  /*                           information about the face; see             */
  /*                           @FT_FACE_FLAG_XXX for the details.          */
  /*                                                                       */
  /*    style_flags         :: A set of bit flags indicating the style of  */
  /*                           the face; see @FT_STYLE_FLAG_XXX for the    */
  /*                           details.                                    */
  /*                                                                       */
  /*    num_glyphs          :: The number of glyphs in the face.  If the   */
  /*                           face is scalable and has sbits (see         */
  /*                           `num_fixed_sizes'), it is set to the number */
  /*                           of outline glyphs.                          */
  /*                                                                       */
  /*                           For CID-keyed fonts, this value gives the   */
  /*                           highest CID used in the font.               */
  /*                                                                       */
  /*    family_name         :: The face's family name.  This is an ASCII   */
  /*                           string, usually in English, which describes */
  /*                           the typeface's family (like `Times New      */
  /*                           Roman', `Bodoni', `Garamond', etc).  This   */
  /*                           is a least common denominator used to list  */
  /*                           fonts.  Some formats (TrueType & OpenType)  */
  /*                           provide localized and Unicode versions of   */
  /*                           this string.  Applications should use the   */
  /*                           format specific interface to access them.   */
  /*                           Can be NULL (e.g., in fonts embedded in a   */
  /*                           PDF file).                                  */
  /*                                                                       */
  /*    style_name          :: The face's style name.  This is an ASCII    */
  /*                           string, usually in English, which describes */
  /*                           the typeface's style (like `Italic',        */
  /*                           `Bold', `Condensed', etc).  Not all font    */
  /*                           formats provide a style name, so this field */
  /*                           is optional, and can be set to NULL.  As    */
  /*                           for `family_name', some formats provide     */
  /*                           localized and Unicode versions of this      */
  /*                           string.  Applications should use the format */
  /*                           specific interface to access them.          */
  /*                                                                       */
  /*    num_fixed_sizes     :: The number of bitmap strikes in the face.   */
  /*                           Even if the face is scalable, there might   */
  /*                           still be bitmap strikes, which are called   */
  /*                           `sbits' in that case.                       */
  /*                                                                       */
  /*    available_sizes     :: An array of @FT_Bitmap_Size for all bitmap  */
  /*                           strikes in the face.  It is set to NULL if  */
  /*                           there is no bitmap strike.                  */
  /*                                                                       */
  /*    num_charmaps        :: The number of charmaps in the face.         */
  /*                                                                       */
  /*    charmaps            :: An array of the charmaps of the face.       */
  /*                                                                       */
  /*    generic             :: A field reserved for client uses.  See the  */
  /*                           @FT_Generic type description.               */
  /*                                                                       */
  /*    bbox                :: The font bounding box.  Coordinates are     */
  /*                           expressed in font units (see                */
  /*                           `units_per_EM').  The box is large enough   */
  /*                           to contain any glyph from the font.  Thus,  */
  /*                           `bbox.yMax' can be seen as the `maximum     */
  /*                           ascender', and `bbox.yMin' as the `minimum  */
  /*                           descender'.  Only relevant for scalable     */
  /*                           formats.                                    */
  /*                                                                       */
  /*                           Note that the bounding box might be off by  */
  /*                           (at least) one pixel for hinted fonts.  See */
  /*                           @FT_Size_Metrics for further discussion.    */
  /*                                                                       */
  /*    units_per_EM        :: The number of font units per EM square for  */
  /*                           this face.  This is typically 2048 for      */
  /*                           TrueType fonts, and 1000 for Type~1 fonts.  */
  /*                           Only relevant for scalable formats.         */
  /*                                                                       */
  /*    ascender            :: The typographic ascender of the face,       */
  /*                           expressed in font units.  For font formats  */
  /*                           not having this information, it is set to   */
  /*                           `bbox.yMax'.  Only relevant for scalable    */
  /*                           formats.                                    */
  /*                                                                       */
  /*    descender           :: The typographic descender of the face,      */
  /*                           expressed in font units.  For font formats  */
  /*                           not having this information, it is set to   */
  /*                           `bbox.yMin'.  Note that this field is       */
  /*                           usually negative.  Only relevant for        */
  /*                           scalable formats.                           */
  /*                                                                       */
  /*    height              :: The height is the vertical distance         */
  /*                           between two consecutive baselines,          */
  /*                           expressed in font units.  It is always      */
  /*                           positive.  Only relevant for scalable       */
  /*                           formats.                                    */
  /*                                                                       */
  /*    max_advance_width   :: The maximum advance width, in font units,   */
  /*                           for all glyphs in this face.  This can be   */
  /*                           used to make word wrapping computations     */
  /*                           faster.  Only relevant for scalable         */
  /*                           formats.                                    */
  /*                                                                       */
  /*    max_advance_height  :: The maximum advance height, in font units,  */
  /*                           for all glyphs in this face.  This is only  */
  /*                           relevant for vertical layouts, and is set   */
  /*                           to `height' for fonts that do not provide   */
  /*                           vertical metrics.  Only relevant for        */
  /*                           scalable formats.                           */
  /*                                                                       */
  /*    underline_position  :: The position, in font units, of the         */
  /*                           underline line for this face.  It is the    */
  /*                           center of the underlining stem.  Only       */
  /*                           relevant for scalable formats.              */
  /*                                                                       */
  /*    underline_thickness :: The thickness, in font units, of the        */
  /*                           underline for this face.  Only relevant for */
  /*                           scalable formats.                           */
  /*                                                                       */
  /*    glyph               :: The face's associated glyph slot(s).        */
  /*                                                                       */
  /*    size                :: The current active size for this face.      */
  /*                                                                       */
  /*    charmap             :: The current active charmap for this face.   */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Fields may be changed after a call to @FT_Attach_File or           */
  /*    @FT_Attach_Stream.                                                 */
  /*                                                                       */
  typedef struct  FT_FaceRec_
  {
	FT_Long           num_faces;
	FT_Long           face_index;

	FT_Long           face_flags;
	FT_Long           style_flags;

	FT_Long           num_glyphs;

	FT_String*        family_name;
	FT_String*        style_name;

	FT_Int            num_fixed_sizes;
	FT_Bitmap_Size*   available_sizes;

	FT_Int            num_charmaps;
	FT_CharMap*       charmaps;

	FT_Generic        generic;

	/*# The following member variables (down to `underline_thickness') */
	/*# are only relevant to scalable outlines; cf. @FT_Bitmap_Size    */
	/*# for bitmap fonts.                                              */
	FT_BBox           bbox;

	FT_UShort         units_per_EM;
	FT_Short          ascender;
	FT_Short          descender;
	FT_Short          height;

	FT_Short          max_advance_width;
	FT_Short          max_advance_height;

	FT_Short          underline_position;
	FT_Short          underline_thickness;

	FT_GlyphSlot      glyph;
	FT_Size           size;
	FT_CharMap        charmap;

	/*@private begin */

	FT_Driver         driver;
	FT_Memory         memory;
	FT_Stream         stream;

	FT_ListRec        sizes_list;

	FT_Generic        autohint;   /* face-specific auto-hinter data */
	void*             extensions; /* unused                         */

	FT_Face_Internal  internal;

	/*@private end */

  } FT_FaceRec;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_FACE_FLAG_XXX                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A list of bit flags used in the `face_flags' field of the          */
  /*    @FT_FaceRec structure.  They inform client applications of         */
  /*    properties of the corresponding face.                              */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_FACE_FLAG_SCALABLE ::                                           */
  /*      Indicates that the face contains outline glyphs.  This doesn't   */
  /*      prevent bitmap strikes, i.e., a face can have both this and      */
  /*      and @FT_FACE_FLAG_FIXED_SIZES set.                               */
  /*                                                                       */
  /*    FT_FACE_FLAG_FIXED_SIZES ::                                        */
  /*      Indicates that the face contains bitmap strikes.  See also the   */
  /*      `num_fixed_sizes' and `available_sizes' fields of @FT_FaceRec.   */
  /*                                                                       */
  /*    FT_FACE_FLAG_FIXED_WIDTH ::                                        */
  /*      Indicates that the face contains fixed-width characters (like    */
  /*      Courier, Lucido, MonoType, etc.).                                */
  /*                                                                       */
  /*    FT_FACE_FLAG_SFNT ::                                               */
  /*      Indicates that the face uses the `sfnt' storage scheme.  For     */
  /*      now, this means TrueType and OpenType.                           */
  /*                                                                       */
  /*    FT_FACE_FLAG_HORIZONTAL ::                                         */
  /*      Indicates that the face contains horizontal glyph metrics.  This */
  /*      should be set for all common formats.                            */
  /*                                                                       */
  /*    FT_FACE_FLAG_VERTICAL ::                                           */
  /*      Indicates that the face contains vertical glyph metrics.  This   */
  /*      is only available in some formats, not all of them.              */
  /*                                                                       */
  /*    FT_FACE_FLAG_KERNING ::                                            */
  /*      Indicates that the face contains kerning information.  If set,   */
  /*      the kerning distance can be retrieved through the function       */
  /*      @FT_Get_Kerning.  Otherwise the function always return the       */
  /*      vector (0,0).  Note that FreeType doesn't handle kerning data    */
  /*      from the `GPOS' table (as present in some OpenType fonts).       */
  /*                                                                       */
  /*    FT_FACE_FLAG_FAST_GLYPHS ::                                        */
  /*      THIS FLAG IS DEPRECATED.  DO NOT USE OR TEST IT.                 */
  /*                                                                       */
  /*    FT_FACE_FLAG_MULTIPLE_MASTERS ::                                   */
  /*      Indicates that the font contains multiple masters and is capable */
  /*      of interpolating between them.  See the multiple-masters         */
  /*      specific API for details.                                        */
  /*                                                                       */
  /*    FT_FACE_FLAG_GLYPH_NAMES ::                                        */
  /*      Indicates that the font contains glyph names that can be         */
  /*      retrieved through @FT_Get_Glyph_Name.  Note that some TrueType   */
  /*      fonts contain broken glyph name tables.  Use the function        */
  /*      @FT_Has_PS_Glyph_Names when needed.                              */
  /*                                                                       */
  /*    FT_FACE_FLAG_EXTERNAL_STREAM ::                                    */
  /*      Used internally by FreeType to indicate that a face's stream was */
  /*      provided by the client application and should not be destroyed   */
  /*      when @FT_Done_Face is called.  Don't read or test this flag.     */
  /*                                                                       */
  /*    FT_FACE_FLAG_HINTER ::                                             */
  /*      Set if the font driver has a hinting machine of its own.  For    */
  /*      example, with TrueType fonts, it makes sense to use data from    */
  /*      the SFNT `gasp' table only if the native TrueType hinting engine */
  /*      (with the bytecode interpreter) is available and active.         */
  /*                                                                       */
  /*    FT_FACE_FLAG_CID_KEYED ::                                          */
  /*      Set if the font is CID-keyed.  In that case, the font is not     */
  /*      accessed by glyph indices but by CID values.  For subsetted      */
  /*      CID-keyed fonts this has the consequence that not all index      */
  /*      values are a valid argument to FT_Load_Glyph.  Only the CID      */
  /*      values for which corresponding glyphs in the subsetted font      */
  /*      exist make FT_Load_Glyph return successfully; in all other cases */
  /*      you get an `FT_Err_Invalid_Argument' error.                      */
  /*                                                                       */
  /*      Note that CID-keyed fonts which are in an SFNT wrapper don't     */
  /*      have this flag set since the glyphs are accessed in the normal   */
  /*      way (using contiguous indices); the `CID-ness' isn't visible to  */
  /*      the application.                                                 */
  /*                                                                       */
  /*    FT_FACE_FLAG_TRICKY ::                                             */
  /*      Set if the font is `tricky', this is, it always needs the        */
  /*      font format's native hinting engine to get a reasonable result.  */
  /*      A typical example is the Chinese font `mingli.ttf' which uses    */
  /*      TrueType bytecode instructions to move and scale all of its      */
  /*      subglyphs.                                                       */
  /*                                                                       */
  /*      It is not possible to autohint such fonts using                  */
  /*      @FT_LOAD_FORCE_AUTOHINT; it will also ignore                     */
  /*      @FT_LOAD_NO_HINTING.  You have to set both @FT_LOAD_NO_HINTING   */
  /*      and @FT_LOAD_NO_AUTOHINT to really disable hinting; however, you */
  /*      probably never want this except for demonstration purposes.      */
  /*                                                                       */
  /*      Currently, there are about a dozen TrueType fonts in the list of */
  /*      tricky fonts; they are hard-coded in file `ttobjs.c'.            */
  /*                                                                       */
#define FT_FACE_FLAG_SCALABLE          ( 1L <<  0 )
#define FT_FACE_FLAG_FIXED_SIZES       ( 1L <<  1 )
#define FT_FACE_FLAG_FIXED_WIDTH       ( 1L <<  2 )
#define FT_FACE_FLAG_SFNT              ( 1L <<  3 )
#define FT_FACE_FLAG_HORIZONTAL        ( 1L <<  4 )
#define FT_FACE_FLAG_VERTICAL          ( 1L <<  5 )
#define FT_FACE_FLAG_KERNING           ( 1L <<  6 )
#define FT_FACE_FLAG_FAST_GLYPHS       ( 1L <<  7 )
#define FT_FACE_FLAG_MULTIPLE_MASTERS  ( 1L <<  8 )
#define FT_FACE_FLAG_GLYPH_NAMES       ( 1L <<  9 )
#define FT_FACE_FLAG_EXTERNAL_STREAM   ( 1L << 10 )
#define FT_FACE_FLAG_HINTER            ( 1L << 11 )
#define FT_FACE_FLAG_CID_KEYED         ( 1L << 12 )
#define FT_FACE_FLAG_TRICKY            ( 1L << 13 )

  /*************************************************************************
   *
   * @macro:
   *   FT_HAS_HORIZONTAL( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains
   *   horizontal metrics (this is true for all font formats though).
   *
   * @also:
   *   @FT_HAS_VERTICAL can be used to check for vertical metrics.
   *
   */
#define FT_HAS_HORIZONTAL( face ) \
		  ( face->face_flags & FT_FACE_FLAG_HORIZONTAL )

  /*************************************************************************
   *
   * @macro:
   *   FT_HAS_VERTICAL( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains vertical
   *   metrics.
   *
   */
#define FT_HAS_VERTICAL( face ) \
		  ( face->face_flags & FT_FACE_FLAG_VERTICAL )

  /*************************************************************************
   *
   * @macro:
   *   FT_HAS_KERNING( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains kerning
   *   data that can be accessed with @FT_Get_Kerning.
   *
   */
#define FT_HAS_KERNING( face ) \
		  ( face->face_flags & FT_FACE_FLAG_KERNING )

  /*************************************************************************
   *
   * @macro:
   *   FT_IS_SCALABLE( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains a scalable
   *   font face (true for TrueType, Type~1, Type~42, CID, OpenType/CFF,
   *   and PFR font formats.
   *
   */
#define FT_IS_SCALABLE( face ) \
		  ( face->face_flags & FT_FACE_FLAG_SCALABLE )

  /*************************************************************************
   *
   * @macro:
   *   FT_IS_SFNT( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains a font
   *   whose format is based on the SFNT storage scheme.  This usually
   *   means: TrueType fonts, OpenType fonts, as well as SFNT-based embedded
   *   bitmap fonts.
   *
   *   If this macro is true, all functions defined in @FT_SFNT_NAMES_H and
   *   @FT_TRUETYPE_TABLES_H are available.
   *
   */
#define FT_IS_SFNT( face ) \
		  ( face->face_flags & FT_FACE_FLAG_SFNT )

  /*************************************************************************
   *
   * @macro:
   *   FT_IS_FIXED_WIDTH( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains a font face
   *   that contains fixed-width (or `monospace', `fixed-pitch', etc.)
   *   glyphs.
   *
   */
#define FT_IS_FIXED_WIDTH( face ) \
		  ( face->face_flags & FT_FACE_FLAG_FIXED_WIDTH )

  /*************************************************************************
   *
   * @macro:
   *   FT_HAS_FIXED_SIZES( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains some
   *   embedded bitmaps.  See the `available_sizes' field of the
   *   @FT_FaceRec structure.
   *
   */
#define FT_HAS_FIXED_SIZES( face ) \
		  ( face->face_flags & FT_FACE_FLAG_FIXED_SIZES )

  /*************************************************************************
   *
   * @macro:
   *   FT_HAS_FAST_GLYPHS( face )
   *
   * @description:
   *   Deprecated.
   *
   */
#define FT_HAS_FAST_GLYPHS( face )  0

  /*************************************************************************
   *
   * @macro:
   *   FT_HAS_GLYPH_NAMES( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains some glyph
   *   names that can be accessed through @FT_Get_Glyph_Name.
   *
   */
#define FT_HAS_GLYPH_NAMES( face ) \
		  ( face->face_flags & FT_FACE_FLAG_GLYPH_NAMES )

  /*************************************************************************
   *
   * @macro:
   *   FT_HAS_MULTIPLE_MASTERS( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains some
   *   multiple masters.  The functions provided by @FT_MULTIPLE_MASTERS_H
   *   are then available to choose the exact design you want.
   *
   */
#define FT_HAS_MULTIPLE_MASTERS( face ) \
		  ( face->face_flags & FT_FACE_FLAG_MULTIPLE_MASTERS )

  /*************************************************************************
   *
   * @macro:
   *   FT_IS_CID_KEYED( face )
   *
   * @description:
   *   A macro that returns true whenever a face object contains a CID-keyed
   *   font.  See the discussion of @FT_FACE_FLAG_CID_KEYED for more
   *   details.
   *
   *   If this macro is true, all functions defined in @FT_CID_H are
   *   available.
   *
   */
#define FT_IS_CID_KEYED( face ) \
		  ( face->face_flags & FT_FACE_FLAG_CID_KEYED )

  /*************************************************************************
   *
   * @macro:
   *   FT_IS_TRICKY( face )
   *
   * @description:
   *   A macro that returns true whenever a face represents a `tricky' font.
   *   See the discussion of @FT_FACE_FLAG_TRICKY for more details.
   *
   */
#define FT_IS_TRICKY( face ) \
		  ( face->face_flags & FT_FACE_FLAG_TRICKY )

  /*************************************************************************/
  /*                                                                       */
  /* <Const>                                                               */
  /*    FT_STYLE_FLAG_XXX                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A list of bit-flags used to indicate the style of a given face.    */
  /*    These are used in the `style_flags' field of @FT_FaceRec.          */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_STYLE_FLAG_ITALIC ::                                            */
  /*      Indicates that a given face style is italic or oblique.          */
  /*                                                                       */
  /*    FT_STYLE_FLAG_BOLD ::                                              */
  /*      Indicates that a given face is bold.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The style information as provided by FreeType is very basic.  More */
  /*    details are beyond the scope and should be done on a higher level  */
  /*    (for example, by analyzing various fields of the `OS/2' table in   */
  /*    SFNT based fonts).                                                 */
  /*                                                                       */
#define FT_STYLE_FLAG_ITALIC  ( 1 << 0 )
#define FT_STYLE_FLAG_BOLD    ( 1 << 1 )

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Size_Internal                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An opaque handle to an `FT_Size_InternalRec' structure, used to    */
  /*    model private data of a given @FT_Size object.                     */
  /*                                                                       */
  typedef struct FT_Size_InternalRec_*  FT_Size_Internal;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Size_Metrics                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The size metrics structure gives the metrics of a size object.     */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x_ppem       :: The width of the scaled EM square in pixels, hence */
  /*                    the term `ppem' (pixels per EM).  It is also       */
  /*                    referred to as `nominal width'.                    */
  /*                                                                       */
  /*    y_ppem       :: The height of the scaled EM square in pixels,      */
  /*                    hence the term `ppem' (pixels per EM).  It is also */
  /*                    referred to as `nominal height'.                   */
  /*                                                                       */
  /*    x_scale      :: A 16.16 fractional scaling value used to convert   */
  /*                    horizontal metrics from font units to 26.6         */
  /*                    fractional pixels.  Only relevant for scalable     */
  /*                    font formats.                                      */
  /*                                                                       */
  /*    y_scale      :: A 16.16 fractional scaling value used to convert   */
  /*                    vertical metrics from font units to 26.6           */
  /*                    fractional pixels.  Only relevant for scalable     */
  /*                    font formats.                                      */
  /*                                                                       */
  /*    ascender     :: The ascender in 26.6 fractional pixels.  See       */
  /*                    @FT_FaceRec for the details.                       */
  /*                                                                       */
  /*    descender    :: The descender in 26.6 fractional pixels.  See      */
  /*                    @FT_FaceRec for the details.                       */
  /*                                                                       */
  /*    height       :: The height in 26.6 fractional pixels.  See         */
  /*                    @FT_FaceRec for the details.                       */
  /*                                                                       */
  /*    max_advance  :: The maximum advance width in 26.6 fractional       */
  /*                    pixels.  See @FT_FaceRec for the details.          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The scaling values, if relevant, are determined first during a     */
  /*    size changing operation.  The remaining fields are then set by the */
  /*    driver.  For scalable formats, they are usually set to scaled      */
  /*    values of the corresponding fields in @FT_FaceRec.                 */
  /*                                                                       */
  /*    Note that due to glyph hinting, these values might not be exact    */
  /*    for certain fonts.  Thus they must be treated as unreliable        */
  /*    with an error margin of at least one pixel!                        */
  /*                                                                       */
  /*    Indeed, the only way to get the exact metrics is to render _all_   */
  /*    glyphs.  As this would be a definite performance hit, it is up to  */
  /*    client applications to perform such computations.                  */
  /*                                                                       */
  /*    The FT_Size_Metrics structure is valid for bitmap fonts also.      */
  /*                                                                       */
  typedef struct  FT_Size_Metrics_
  {
	FT_UShort  x_ppem;      /* horizontal pixels per EM               */
	FT_UShort  y_ppem;      /* vertical pixels per EM                 */

	FT_Fixed   x_scale;     /* scaling values used to convert font    */
	FT_Fixed   y_scale;     /* units to 26.6 fractional pixels        */

	FT_Pos     ascender;    /* ascender in 26.6 frac. pixels          */
	FT_Pos     descender;   /* descender in 26.6 frac. pixels         */
	FT_Pos     height;      /* text height in 26.6 frac. pixels       */
	FT_Pos     max_advance; /* max horizontal advance, in 26.6 pixels */

  } FT_Size_Metrics;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_SizeRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    FreeType root size class structure.  A size object models a face   */
  /*    object at a given size.                                            */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    face    :: Handle to the parent face object.                       */
  /*                                                                       */
  /*    generic :: A typeless pointer, which is unused by the FreeType     */
  /*               library or any of its drivers.  It can be used by       */
  /*               client applications to link their own data to each size */
  /*               object.                                                 */
  /*                                                                       */
  /*    metrics :: Metrics for this size object.  This field is read-only. */
  /*                                                                       */
  typedef struct  FT_SizeRec_
  {
	FT_Face           face;      /* parent face object              */
	FT_Generic        generic;   /* generic pointer for client uses */
	FT_Size_Metrics   metrics;   /* size metrics                    */
	FT_Size_Internal  internal;

  } FT_SizeRec;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_SubGlyph                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The subglyph structure is an internal object used to describe      */
  /*    subglyphs (for example, in the case of composites).                */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The subglyph implementation is not part of the high-level API,     */
  /*    hence the forward structure declaration.                           */
  /*                                                                       */
  /*    You can however retrieve subglyph information with                 */
  /*    @FT_Get_SubGlyph_Info.                                             */
  /*                                                                       */
  typedef struct FT_SubGlyphRec_*  FT_SubGlyph;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Slot_Internal                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An opaque handle to an `FT_Slot_InternalRec' structure, used to    */
  /*    model private data of a given @FT_GlyphSlot object.                */
  /*                                                                       */
  typedef struct FT_Slot_InternalRec_*  FT_Slot_Internal;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_GlyphSlotRec                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    FreeType root glyph slot class structure.  A glyph slot is a       */
  /*    container where individual glyphs can be loaded, be they in        */
  /*    outline or bitmap format.                                          */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    library           :: A handle to the FreeType library instance     */
  /*                         this slot belongs to.                         */
  /*                                                                       */
  /*    face              :: A handle to the parent face object.           */
  /*                                                                       */
  /*    next              :: In some cases (like some font tools), several */
  /*                         glyph slots per face object can be a good     */
  /*                         thing.  As this is rare, the glyph slots are  */
  /*                         listed through a direct, single-linked list   */
  /*                         using its `next' field.                       */
  /*                                                                       */
  /*    generic           :: A typeless pointer which is unused by the     */
  /*                         FreeType library or any of its drivers.  It   */
  /*                         can be used by client applications to link    */
  /*                         their own data to each glyph slot object.     */
  /*                                                                       */
  /*    metrics           :: The metrics of the last loaded glyph in the   */
  /*                         slot.  The returned values depend on the last */
  /*                         load flags (see the @FT_Load_Glyph API        */
  /*                         function) and can be expressed either in 26.6 */
  /*                         fractional pixels or font units.              */
  /*                                                                       */
  /*                         Note that even when the glyph image is        */
  /*                         transformed, the metrics are not.             */
  /*                                                                       */
  /*    linearHoriAdvance :: The advance width of the unhinted glyph.      */
  /*                         Its value is expressed in 16.16 fractional    */
  /*                         pixels, unless @FT_LOAD_LINEAR_DESIGN is set  */
  /*                         when loading the glyph.  This field can be    */
  /*                         important to perform correct WYSIWYG layout.  */
  /*                         Only relevant for outline glyphs.             */
  /*                                                                       */
  /*    linearVertAdvance :: The advance height of the unhinted glyph.     */
  /*                         Its value is expressed in 16.16 fractional    */
  /*                         pixels, unless @FT_LOAD_LINEAR_DESIGN is set  */
  /*                         when loading the glyph.  This field can be    */
  /*                         important to perform correct WYSIWYG layout.  */
  /*                         Only relevant for outline glyphs.             */
  /*                                                                       */
  /*    advance           :: This shorthand is, depending on               */
  /*                         @FT_LOAD_IGNORE_TRANSFORM, the transformed    */
  /*                         advance width for the glyph (in 26.6          */
  /*                         fractional pixel format).  As specified with  */
  /*                         @FT_LOAD_VERTICAL_LAYOUT, it uses either the  */
  /*                         `horiAdvance' or the `vertAdvance' value of   */
  /*                         `metrics' field.                              */
  /*                                                                       */
  /*    format            :: This field indicates the format of the image  */
  /*                         contained in the glyph slot.  Typically       */
  /*                         @FT_GLYPH_FORMAT_BITMAP,                      */
  /*                         @FT_GLYPH_FORMAT_OUTLINE, or                  */
  /*                         @FT_GLYPH_FORMAT_COMPOSITE, but others are    */
  /*                         possible.                                     */
  /*                                                                       */
  /*    bitmap            :: This field is used as a bitmap descriptor     */
  /*                         when the slot format is                       */
  /*                         @FT_GLYPH_FORMAT_BITMAP.  Note that the       */
  /*                         address and content of the bitmap buffer can  */
  /*                         change between calls of @FT_Load_Glyph and a  */
  /*                         few other functions.                          */
  /*                                                                       */
  /*    bitmap_left       :: This is the bitmap's left bearing expressed   */
  /*                         in integer pixels.  Of course, this is only   */
  /*                         valid if the format is                        */
  /*                         @FT_GLYPH_FORMAT_BITMAP.                      */
  /*                                                                       */
  /*    bitmap_top        :: This is the bitmap's top bearing expressed in */
  /*                         integer pixels.  Remember that this is the    */
  /*                         distance from the baseline to the top-most    */
  /*                         glyph scanline, upwards y~coordinates being   */
  /*                         *positive*.                                   */
  /*                                                                       */
  /*    outline           :: The outline descriptor for the current glyph  */
  /*                         image if its format is                        */
  /*                         @FT_GLYPH_FORMAT_OUTLINE.  Once a glyph is    */
  /*                         loaded, `outline' can be transformed,         */
  /*                         distorted, embolded, etc.  However, it must   */
  /*                         not be freed.                                 */
  /*                                                                       */
  /*    num_subglyphs     :: The number of subglyphs in a composite glyph. */
  /*                         This field is only valid for the composite    */
  /*                         glyph format that should normally only be     */
  /*                         loaded with the @FT_LOAD_NO_RECURSE flag.     */
  /*                         For now this is internal to FreeType.         */
  /*                                                                       */
  /*    subglyphs         :: An array of subglyph descriptors for          */
  /*                         composite glyphs.  There are `num_subglyphs'  */
  /*                         elements in there.  Currently internal to     */
  /*                         FreeType.                                     */
  /*                                                                       */
  /*    control_data      :: Certain font drivers can also return the      */
  /*                         control data for a given glyph image (e.g.    */
  /*                         TrueType bytecode, Type~1 charstrings, etc.). */
  /*                         This field is a pointer to such data.         */
  /*                                                                       */
  /*    control_len       :: This is the length in bytes of the control    */
  /*                         data.                                         */
  /*                                                                       */
  /*    other             :: Really wicked formats can use this pointer to */
  /*                         present their own glyph image to client       */
  /*                         applications.  Note that the application      */
  /*                         needs to know about the image format.         */
  /*                                                                       */
  /*    lsb_delta         :: The difference between hinted and unhinted    */
  /*                         left side bearing while autohinting is        */
  /*                         active.  Zero otherwise.                      */
  /*                                                                       */
  /*    rsb_delta         :: The difference between hinted and unhinted    */
  /*                         right side bearing while autohinting is       */
  /*                         active.  Zero otherwise.                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If @FT_Load_Glyph is called with default flags (see                */
  /*    @FT_LOAD_DEFAULT) the glyph image is loaded in the glyph slot in   */
  /*    its native format (e.g., an outline glyph for TrueType and Type~1  */
  /*    formats).                                                          */
  /*                                                                       */
  /*    This image can later be converted into a bitmap by calling         */
  /*    @FT_Render_Glyph.  This function finds the current renderer for    */
  /*    the native image's format, then invokes it.                        */
  /*                                                                       */
  /*    The renderer is in charge of transforming the native image through */
  /*    the slot's face transformation fields, then converting it into a   */
  /*    bitmap that is returned in `slot->bitmap'.                         */
  /*                                                                       */
  /*    Note that `slot->bitmap_left' and `slot->bitmap_top' are also used */
  /*    to specify the position of the bitmap relative to the current pen  */
  /*    position (e.g., coordinates (0,0) on the baseline).  Of course,    */
  /*    `slot->format' is also changed to @FT_GLYPH_FORMAT_BITMAP.         */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Here a small pseudo code fragment which shows how to use           */
  /*    `lsb_delta' and `rsb_delta':                                       */
  /*                                                                       */
  /*    {                                                                  */
  /*      FT_Pos  origin_x       = 0;                                      */
  /*      FT_Pos  prev_rsb_delta = 0;                                      */
  /*                                                                       */
  /*                                                                       */
  /*      for all glyphs do                                                */
  /*        <compute kern between current and previous glyph and add it to */
  /*         `origin_x'>                                                   */
  /*                                                                       */
  /*        <load glyph with `FT_Load_Glyph'>                              */
  /*                                                                       */
  /*        if ( prev_rsb_delta - face->glyph->lsb_delta >= 32 )           */
  /*          origin_x -= 64;                                              */
  /*        else if ( prev_rsb_delta - face->glyph->lsb_delta < -32 )      */
  /*          origin_x += 64;                                              */
  /*                                                                       */
  /*        prev_rsb_delta = face->glyph->rsb_delta;                       */
  /*                                                                       */
  /*        <save glyph image, or render glyph, or ...>                    */
  /*                                                                       */
  /*        origin_x += face->glyph->advance.x;                            */
  /*      endfor                                                           */
  /*    }                                                                  */
  /*                                                                       */
  typedef struct  FT_GlyphSlotRec_
  {
	FT_Library        library;
	FT_Face           face;
	FT_GlyphSlot      next;
	FT_UInt           reserved;       /* retained for binary compatibility */
	FT_Generic        generic;

	FT_Glyph_Metrics  metrics;
	FT_Fixed          linearHoriAdvance;
	FT_Fixed          linearVertAdvance;
	FT_Vector         advance;

	FT_Glyph_Format   format;

	FT_Bitmap         bitmap;
	FT_Int            bitmap_left;
	FT_Int            bitmap_top;

	FT_Outline        outline;

	FT_UInt           num_subglyphs;
	FT_SubGlyph       subglyphs;

	void*             control_data;
	long              control_len;

	FT_Pos            lsb_delta;
	FT_Pos            rsb_delta;

	void*             other;

	FT_Slot_Internal  internal;

  } FT_GlyphSlotRec;

  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                         F U N C T I O N S                             */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Init_FreeType                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initialize a new FreeType library object.  The set of modules      */
  /*    that are registered by this function is determined at build time.  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    alibrary :: A handle to a new library object.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    In case you want to provide your own memory allocating routines,   */
  /*    use @FT_New_Library instead, followed by a call to                 */
  /*    @FT_Add_Default_Modules (or a series of calls to @FT_Add_Module).  */
  /*                                                                       */
  /*    For multi-threading applications each thread should have its own   */
  /*    FT_Library object.                                                 */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Init_FreeType( FT_Library  *alibrary );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_FreeType                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroy a given FreeType library object and all of its children,   */
  /*    including resources, drivers, faces, sizes, etc.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to the target library object.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Done_FreeType( FT_Library  library );

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_OPEN_XXX                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A list of bit-field constants used within the `flags' field of the */
  /*    @FT_Open_Args structure.                                           */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_OPEN_MEMORY   :: This is a memory-based stream.                 */
  /*                                                                       */
  /*    FT_OPEN_STREAM   :: Copy the stream from the `stream' field.       */
  /*                                                                       */
  /*    FT_OPEN_PATHNAME :: Create a new input stream from a C~path        */
  /*                        name.                                          */
  /*                                                                       */
  /*    FT_OPEN_DRIVER   :: Use the `driver' field.                        */
  /*                                                                       */
  /*    FT_OPEN_PARAMS   :: Use the `num_params' and `params' fields.      */
  /*                                                                       */
  /*    ft_open_memory   :: Deprecated; use @FT_OPEN_MEMORY instead.       */
  /*                                                                       */
  /*    ft_open_stream   :: Deprecated; use @FT_OPEN_STREAM instead.       */
  /*                                                                       */
  /*    ft_open_pathname :: Deprecated; use @FT_OPEN_PATHNAME instead.     */
  /*                                                                       */
  /*    ft_open_driver   :: Deprecated; use @FT_OPEN_DRIVER instead.       */
  /*                                                                       */
  /*    ft_open_params   :: Deprecated; use @FT_OPEN_PARAMS instead.       */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The `FT_OPEN_MEMORY', `FT_OPEN_STREAM', and `FT_OPEN_PATHNAME'     */
  /*    flags are mutually exclusive.                                      */
  /*                                                                       */
#define FT_OPEN_MEMORY    0x1
#define FT_OPEN_STREAM    0x2
#define FT_OPEN_PATHNAME  0x4
#define FT_OPEN_DRIVER    0x8
#define FT_OPEN_PARAMS    0x10

#define ft_open_memory    FT_OPEN_MEMORY     /* deprecated */
#define ft_open_stream    FT_OPEN_STREAM     /* deprecated */
#define ft_open_pathname  FT_OPEN_PATHNAME   /* deprecated */
#define ft_open_driver    FT_OPEN_DRIVER     /* deprecated */
#define ft_open_params    FT_OPEN_PARAMS     /* deprecated */

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Parameter                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to pass more or less generic parameters to */
  /*    @FT_Open_Face.                                                     */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    tag  :: A four-byte identification tag.                            */
  /*                                                                       */
  /*    data :: A pointer to the parameter data.                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The ID and function of parameters are driver-specific.  See the    */
  /*    various FT_PARAM_TAG_XXX flags for more information.               */
  /*                                                                       */
  typedef struct  FT_Parameter_
  {
	FT_ULong    tag;
	FT_Pointer  data;

  } FT_Parameter;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Open_Args                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to indicate how to open a new font file or        */
  /*    stream.  A pointer to such a structure can be used as a parameter  */
  /*    for the functions @FT_Open_Face and @FT_Attach_Stream.             */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    flags       :: A set of bit flags indicating how to use the        */
  /*                   structure.                                          */
  /*                                                                       */
  /*    memory_base :: The first byte of the file in memory.               */
  /*                                                                       */
  /*    memory_size :: The size in bytes of the file in memory.            */
  /*                                                                       */
  /*    pathname    :: A pointer to an 8-bit file pathname.                */
  /*                                                                       */
  /*    stream      :: A handle to a source stream object.                 */
  /*                                                                       */
  /*    driver      :: This field is exclusively used by @FT_Open_Face;    */
  /*                   it simply specifies the font driver to use to open  */
  /*                   the face.  If set to~0, FreeType tries to load the  */
  /*                   face with each one of the drivers in its list.      */
  /*                                                                       */
  /*    num_params  :: The number of extra parameters.                     */
  /*                                                                       */
  /*    params      :: Extra parameters passed to the font driver when     */
  /*                   opening a new face.                                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The stream type is determined by the contents of `flags' which     */
  /*    are tested in the following order by @FT_Open_Face:                */
  /*                                                                       */
  /*    If the `FT_OPEN_MEMORY' bit is set, assume that this is a          */
  /*    memory file of `memory_size' bytes, located at `memory_address'.   */
  /*    The data are are not copied, and the client is responsible for     */
  /*    releasing and destroying them _after_ the corresponding call to    */
  /*    @FT_Done_Face.                                                     */
  /*                                                                       */
  /*    Otherwise, if the `FT_OPEN_STREAM' bit is set, assume that a       */
  /*    custom input stream `stream' is used.                              */
  /*                                                                       */
  /*    Otherwise, if the `FT_OPEN_PATHNAME' bit is set, assume that this  */
  /*    is a normal file and use `pathname' to open it.                    */
  /*                                                                       */
  /*    If the `FT_OPEN_DRIVER' bit is set, @FT_Open_Face only tries to    */
  /*    open the file with the driver whose handler is in `driver'.        */
  /*                                                                       */
  /*    If the `FT_OPEN_PARAMS' bit is set, the parameters given by        */
  /*    `num_params' and `params' is used.  They are ignored otherwise.    */
  /*                                                                       */
  /*    Ideally, both the `pathname' and `params' fields should be tagged  */
  /*    as `const'; this is missing for API backwards compatibility.  In   */
  /*    other words, applications should treat them as read-only.          */
  /*                                                                       */
  typedef struct  FT_Open_Args_
  {
	FT_UInt         flags;
	const FT_Byte*  memory_base;
	FT_Long         memory_size;
	FT_String*      pathname;
	FT_Stream       stream;
	FT_Module       driver;
	FT_Int          num_params;
	FT_Parameter*   params;

  } FT_Open_Args;

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_Face                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function calls @FT_Open_Face to open a font by its pathname.  */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library resource.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    pathname   :: A path to the font file.                             */
  /*                                                                       */
  /*    face_index :: The index of the face within the font.  The first    */
  /*                  face has index~0.                                    */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aface      :: A handle to a new face object.  If `face_index' is   */
  /*                  greater than or equal to zero, it must be non-NULL.  */
  /*                  See @FT_Open_Face for more details.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_New_Face( FT_Library   library,
			   const char*  filepathname,
			   FT_Long      face_index,
			   FT_Face     *aface );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_New_Memory_Face                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function calls @FT_Open_Face to open a font which has been    */
  /*    loaded into memory.                                                */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library resource.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    file_base  :: A pointer to the beginning of the font data.         */
  /*                                                                       */
  /*    file_size  :: The size of the memory chunk used by the font data.  */
  /*                                                                       */
  /*    face_index :: The index of the face within the font.  The first    */
  /*                  face has index~0.                                    */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aface      :: A handle to a new face object.  If `face_index' is   */
  /*                  greater than or equal to zero, it must be non-NULL.  */
  /*                  See @FT_Open_Face for more details.                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You must not deallocate the memory before calling @FT_Done_Face.   */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_New_Memory_Face( FT_Library      library,
					  const FT_Byte*  file_base,
					  FT_Long         file_size,
					  FT_Long         face_index,
					  FT_Face        *aface );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Open_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Create a face object from a given resource described by            */
  /*    @FT_Open_Args.                                                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library resource.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    args       :: A pointer to an `FT_Open_Args' structure which must  */
  /*                  be filled by the caller.                             */
  /*                                                                       */
  /*    face_index :: The index of the face within the font.  The first    */
  /*                  face has index~0.                                    */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aface      :: A handle to a new face object.  If `face_index' is   */
  /*                  greater than or equal to zero, it must be non-NULL.  */
  /*                  See note below.                                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Unlike FreeType 1.x, this function automatically creates a glyph   */
  /*    slot for the face object which can be accessed directly through    */
  /*    `face->glyph'.                                                     */
  /*                                                                       */
  /*    FT_Open_Face can be used to quickly check whether the font         */
  /*    format of a given font resource is supported by FreeType.  If the  */
  /*    `face_index' field is negative, the function's return value is~0   */
  /*    if the font format is recognized, or non-zero otherwise;           */
  /*    the function returns a more or less empty face handle in `*aface'  */
  /*    (if `aface' isn't NULL).  The only useful field in this special    */
  /*    case is `face->num_faces' which gives the number of faces within   */
  /*    the font file.  After examination, the returned @FT_Face structure */
  /*    should be deallocated with a call to @FT_Done_Face.                */
  /*                                                                       */
  /*    Each new face object created with this function also owns a        */
  /*    default @FT_Size object, accessible as `face->size'.               */
  /*                                                                       */
  /*    One @FT_Library instance can have multiple face objects, this is,  */
  /*    @FT_Open_Face and its siblings can be called multiple times using  */
  /*    the same `library' argument.                                       */
  /*                                                                       */
  /*    See the discussion of reference counters in the description of     */
  /*    @FT_Reference_Face.                                                */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Open_Face( FT_Library           library,
				const FT_Open_Args*  args,
				FT_Long              face_index,
				FT_Face             *aface );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Attach_File                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function calls @FT_Attach_Stream to attach a file.            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face         :: The target face object.                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    filepathname :: The pathname.                                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Attach_File( FT_Face      face,
				  const char*  filepathname );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Attach_Stream                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    `Attach' data to a face object.  Normally, this is used to read    */
  /*    additional information for the face object.  For example, you can  */
  /*    attach an AFM file that comes with a Type~1 font to get the        */
  /*    kerning values and other metrics.                                  */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face       :: The target face object.                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    parameters :: A pointer to @FT_Open_Args which must be filled by   */
  /*                  the caller.                                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The meaning of the `attach' (i.e., what really happens when the    */
  /*    new file is read) is not fixed by FreeType itself.  It really      */
  /*    depends on the font format (and thus the font driver).             */
  /*                                                                       */
  /*    Client applications are expected to know what they are doing       */
  /*    when invoking this function.  Most drivers simply do not implement */
  /*    file attachments.                                                  */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Attach_Stream( FT_Face        face,
					FT_Open_Args*  parameters );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Reference_Face                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A counter gets initialized to~1 at the time an @FT_Face structure  */
  /*    is created.  This function increments the counter.  @FT_Done_Face  */
  /*    then only destroys a face if the counter is~1, otherwise it simply */
  /*    decrements the counter.                                            */
  /*                                                                       */
  /*    This function helps in managing life-cycles of structures which    */
  /*    reference @FT_Face objects.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to a target face object.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.4.2                                                              */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Reference_Face( FT_Face  face );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_Face                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Discard a given face object, as well as all of its child slots and */
  /*    sizes.                                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to a target face object.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    See the discussion of reference counters in the description of     */
  /*    @FT_Reference_Face.                                                */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Done_Face( FT_Face  face );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Select_Size                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Select a bitmap strike.                                            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face         :: A handle to a target face object.                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    strike_index :: The index of the bitmap strike in the              */
  /*                    `available_sizes' field of @FT_FaceRec structure.  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Select_Size( FT_Face  face,
				  FT_Int   strike_index );

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Size_Request_Type                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration type that lists the supported size request types.   */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_SIZE_REQUEST_TYPE_NOMINAL ::                                    */
  /*      The nominal size.  The `units_per_EM' field of @FT_FaceRec is    */
  /*      used to determine both scaling values.                           */
  /*                                                                       */
  /*    FT_SIZE_REQUEST_TYPE_REAL_DIM ::                                   */
  /*      The real dimension.  The sum of the the `ascender' and (minus    */
  /*      of) the `descender' fields of @FT_FaceRec are used to determine  */
  /*      both scaling values.                                             */
  /*                                                                       */
  /*    FT_SIZE_REQUEST_TYPE_BBOX ::                                       */
  /*      The font bounding box.  The width and height of the `bbox' field */
  /*      of @FT_FaceRec are used to determine the horizontal and vertical */
  /*      scaling value, respectively.                                     */
  /*                                                                       */
  /*    FT_SIZE_REQUEST_TYPE_CELL ::                                       */
  /*      The `max_advance_width' field of @FT_FaceRec is used to          */
  /*      determine the horizontal scaling value; the vertical scaling     */
  /*      value is determined the same way as                              */
  /*      @FT_SIZE_REQUEST_TYPE_REAL_DIM does.  Finally, both scaling      */
  /*      values are set to the smaller one.  This type is useful if you   */
  /*      want to specify the font size for, say, a window of a given      */
  /*      dimension and 80x24 cells.                                       */
  /*                                                                       */
  /*    FT_SIZE_REQUEST_TYPE_SCALES ::                                     */
  /*      Specify the scaling values directly.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The above descriptions only apply to scalable formats.  For bitmap */
  /*    formats, the behaviour is up to the driver.                        */
  /*                                                                       */
  /*    See the note section of @FT_Size_Metrics if you wonder how size    */
  /*    requesting relates to scaling values.                              */
  /*                                                                       */
  typedef enum  FT_Size_Request_Type_
  {
	FT_SIZE_REQUEST_TYPE_NOMINAL,
	FT_SIZE_REQUEST_TYPE_REAL_DIM,
	FT_SIZE_REQUEST_TYPE_BBOX,
	FT_SIZE_REQUEST_TYPE_CELL,
	FT_SIZE_REQUEST_TYPE_SCALES,

	FT_SIZE_REQUEST_TYPE_MAX

  } FT_Size_Request_Type;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Size_RequestRec                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model a size request.                          */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    type           :: See @FT_Size_Request_Type.                       */
  /*                                                                       */
  /*    width          :: The desired width.                               */
  /*                                                                       */
  /*    height         :: The desired height.                              */
  /*                                                                       */
  /*    horiResolution :: The horizontal resolution.  If set to zero,      */
  /*                      `width' is treated as a 26.6 fractional pixel    */
  /*                      value.                                           */
  /*                                                                       */
  /*    vertResolution :: The vertical resolution.  If set to zero,        */
  /*                      `height' is treated as a 26.6 fractional pixel   */
  /*                      value.                                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If `width' is zero, then the horizontal scaling value is set equal */
  /*    to the vertical scaling value, and vice versa.                     */
  /*                                                                       */
  typedef struct  FT_Size_RequestRec_
  {
	FT_Size_Request_Type  type;
	FT_Long               width;
	FT_Long               height;
	FT_UInt               horiResolution;
	FT_UInt               vertResolution;

  } FT_Size_RequestRec;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Size_Request                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a size request structure.                              */
  /*                                                                       */
  typedef struct FT_Size_RequestRec_  *FT_Size_Request;

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Request_Size                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Resize the scale of the active @FT_Size object in a face.          */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face :: A handle to a target face object.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    req  :: A pointer to a @FT_Size_RequestRec.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Although drivers may select the bitmap strike matching the         */
  /*    request, you should not rely on this if you intend to select a     */
  /*    particular bitmap strike.  Use @FT_Select_Size instead in that     */
  /*    case.                                                              */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Request_Size( FT_Face          face,
				   FT_Size_Request  req );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Char_Size                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function calls @FT_Request_Size to request the nominal size   */
  /*    (in points).                                                       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face            :: A handle to a target face object.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    char_width      :: The nominal width, in 26.6 fractional points.   */
  /*                                                                       */
  /*    char_height     :: The nominal height, in 26.6 fractional points.  */
  /*                                                                       */
  /*    horz_resolution :: The horizontal resolution in dpi.               */
  /*                                                                       */
  /*    vert_resolution :: The vertical resolution in dpi.                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If either the character width or height is zero, it is set equal   */
  /*    to the other value.                                                */
  /*                                                                       */
  /*    If either the horizontal or vertical resolution is zero, it is set */
  /*    equal to the other value.                                          */
  /*                                                                       */
  /*    A character width or height smaller than 1pt is set to 1pt; if     */
  /*    both resolution values are zero, they are set to 72dpi.            */
  /*                                                                       */
  /*    Don't use this function if you are using the FreeType cache API.   */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Set_Char_Size( FT_Face     face,
					FT_F26Dot6  char_width,
					FT_F26Dot6  char_height,
					FT_UInt     horz_resolution,
					FT_UInt     vert_resolution );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Pixel_Sizes                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function calls @FT_Request_Size to request the nominal size   */
  /*    (in pixels).                                                       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face         :: A handle to the target face object.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    pixel_width  :: The nominal width, in pixels.                      */
  /*                                                                       */
  /*    pixel_height :: The nominal height, in pixels.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Set_Pixel_Sizes( FT_Face  face,
					  FT_UInt  pixel_width,
					  FT_UInt  pixel_height );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Load_Glyph                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to load a single glyph into the glyph slot of a    */
  /*    face object.                                                       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face        :: A handle to the target face object where the glyph  */
  /*                   is loaded.                                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    glyph_index :: The index of the glyph in the font file.  For       */
  /*                   CID-keyed fonts (either in PS or in CFF format)     */
  /*                   this argument specifies the CID value.              */
  /*                                                                       */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   @FT_LOAD_XXX constants can be used to control the   */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The loaded glyph may be transformed.  See @FT_Set_Transform for    */
  /*    the details.                                                       */
  /*                                                                       */
  /*    For subsetted CID-keyed fonts, `FT_Err_Invalid_Argument' is        */
  /*    returned for invalid CID values (this is, for CID values which     */
  /*    don't have a corresponding glyph in the font).  See the discussion */
  /*    of the @FT_FACE_FLAG_CID_KEYED flag for more details.              */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Load_Glyph( FT_Face   face,
				 FT_UInt   glyph_index,
				 FT_Int32  load_flags );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Load_Char                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to load a single glyph into the glyph slot of a    */
  /*    face object, according to its character code.                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face        :: A handle to a target face object where the glyph    */
  /*                   is loaded.                                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    char_code   :: The glyph's character code, according to the        */
  /*                   current charmap used in the face.                   */
  /*                                                                       */
  /*    load_flags  :: A flag indicating what to load for this glyph.  The */
  /*                   @FT_LOAD_XXX constants can be used to control the   */
  /*                   glyph loading process (e.g., whether the outline    */
  /*                   should be scaled, whether to load bitmaps or not,   */
  /*                   whether to hint the outline, etc).                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function simply calls @FT_Get_Char_Index and @FT_Load_Glyph.  */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Load_Char( FT_Face   face,
				FT_ULong  char_code,
				FT_Int32  load_flags );

  /*************************************************************************
   *
   * @enum:
   *   FT_LOAD_XXX
   *
   * @description:
   *   A list of bit-field constants used with @FT_Load_Glyph to indicate
   *   what kind of operations to perform during glyph loading.
   *
   * @values:
   *   FT_LOAD_DEFAULT ::
   *     Corresponding to~0, this value is used as the default glyph load
   *     operation.  In this case, the following happens:
   *
   *     1. FreeType looks for a bitmap for the glyph corresponding to the
   *        face's current size.  If one is found, the function returns.
   *        The bitmap data can be accessed from the glyph slot (see note
   *        below).
   *
   *     2. If no embedded bitmap is searched or found, FreeType looks for a
   *        scalable outline.  If one is found, it is loaded from the font
   *        file, scaled to device pixels, then `hinted' to the pixel grid
   *        in order to optimize it.  The outline data can be accessed from
   *        the glyph slot (see note below).
   *
   *     Note that by default, the glyph loader doesn't render outlines into
   *     bitmaps.  The following flags are used to modify this default
   *     behaviour to more specific and useful cases.
   *
   *   FT_LOAD_NO_SCALE ::
   *     Don't scale the outline glyph loaded, but keep it in font units.
   *
   *     This flag implies @FT_LOAD_NO_HINTING and @FT_LOAD_NO_BITMAP, and
   *     unsets @FT_LOAD_RENDER.
   *
   *   FT_LOAD_NO_HINTING ::
   *     Disable hinting.  This generally generates `blurrier' bitmap glyph
   *     when the glyph is rendered in any of the anti-aliased modes.  See
   *     also the note below.
   *
   *     This flag is implied by @FT_LOAD_NO_SCALE.
   *
   *   FT_LOAD_RENDER ::
   *     Call @FT_Render_Glyph after the glyph is loaded.  By default, the
   *     glyph is rendered in @FT_RENDER_MODE_NORMAL mode.  This can be
   *     overridden by @FT_LOAD_TARGET_XXX or @FT_LOAD_MONOCHROME.
   *
   *     This flag is unset by @FT_LOAD_NO_SCALE.
   *
   *   FT_LOAD_NO_BITMAP ::
   *     Ignore bitmap strikes when loading.  Bitmap-only fonts ignore this
   *     flag.
   *
   *     @FT_LOAD_NO_SCALE always sets this flag.
   *
   *   FT_LOAD_VERTICAL_LAYOUT ::
   *     Load the glyph for vertical text layout.  _Don't_ use it as it is
   *     problematic currently.
   *
   *   FT_LOAD_FORCE_AUTOHINT ::
   *     Indicates that the auto-hinter is preferred over the font's native
   *     hinter.  See also the note below.
   *
   *   FT_LOAD_CROP_BITMAP ::
   *     Indicates that the font driver should crop the loaded bitmap glyph
   *     (i.e., remove all space around its black bits).  Not all drivers
   *     implement this.
   *
   *   FT_LOAD_PEDANTIC ::
   *     Indicates that the font driver should perform pedantic verifications
   *     during glyph loading.  This is mostly used to detect broken glyphs
   *     in fonts.  By default, FreeType tries to handle broken fonts also.
   *
   *     In particular, errors from the TrueType bytecode engine are not
   *     passed to the application if this flag is not set; this might
   *     result in partially hinted or distorted glyphs in case a glyph's
   *     bytecode is buggy.
   *
   *   FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH ::
   *     Ignored.  Deprecated.
   *
   *   FT_LOAD_NO_RECURSE ::
   *     This flag is only used internally.  It merely indicates that the
   *     font driver should not load composite glyphs recursively.  Instead,
   *     it should set the `num_subglyph' and `subglyphs' values of the
   *     glyph slot accordingly, and set `glyph->format' to
   *     @FT_GLYPH_FORMAT_COMPOSITE.
   *
   *     The description of sub-glyphs is not available to client
   *     applications for now.
   *
   *     This flag implies @FT_LOAD_NO_SCALE and @FT_LOAD_IGNORE_TRANSFORM.
   *
   *   FT_LOAD_IGNORE_TRANSFORM ::
   *     Indicates that the transform matrix set by @FT_Set_Transform should
   *     be ignored.
   *
   *   FT_LOAD_MONOCHROME ::
   *     This flag is used with @FT_LOAD_RENDER to indicate that you want to
   *     render an outline glyph to a 1-bit monochrome bitmap glyph, with
   *     8~pixels packed into each byte of the bitmap data.
   *
   *     Note that this has no effect on the hinting algorithm used.  You
   *     should rather use @FT_LOAD_TARGET_MONO so that the
   *     monochrome-optimized hinting algorithm is used.
   *
   *   FT_LOAD_LINEAR_DESIGN ::
   *     Indicates that the `linearHoriAdvance' and `linearVertAdvance'
   *     fields of @FT_GlyphSlotRec should be kept in font units.  See
   *     @FT_GlyphSlotRec for details.
   *
   *   FT_LOAD_NO_AUTOHINT ::
   *     Disable auto-hinter.  See also the note below.
   *
   * @note:
   *   By default, hinting is enabled and the font's native hinter (see
   *   @FT_FACE_FLAG_HINTER) is preferred over the auto-hinter.  You can
   *   disable hinting by setting @FT_LOAD_NO_HINTING or change the
   *   precedence by setting @FT_LOAD_FORCE_AUTOHINT.  You can also set
   *   @FT_LOAD_NO_AUTOHINT in case you don't want the auto-hinter to be
   *   used at all.
   *
   *   See the description of @FT_FACE_FLAG_TRICKY for a special exception
   *   (affecting only a handful of Asian fonts).
   *
   *   Besides deciding which hinter to use, you can also decide which
   *   hinting algorithm to use.  See @FT_LOAD_TARGET_XXX for details.
   *
   *   Note that the auto-hinter needs a valid Unicode cmap (either a native
   *   one or synthesized by FreeType) for producing correct results.  If a
   *   font provides an incorrect mapping (for example, assigning the
   *   character code U+005A, LATIN CAPITAL LETTER Z, to a glyph depicting a
   *   mathematical integral sign), the auto-hinter might produce useless
   *   results.
   *
   */
#define FT_LOAD_DEFAULT                      0x0
#define FT_LOAD_NO_SCALE                     ( 1L << 0 )
#define FT_LOAD_NO_HINTING                   ( 1L << 1 )
#define FT_LOAD_RENDER                       ( 1L << 2 )
#define FT_LOAD_NO_BITMAP                    ( 1L << 3 )
#define FT_LOAD_VERTICAL_LAYOUT              ( 1L << 4 )
#define FT_LOAD_FORCE_AUTOHINT               ( 1L << 5 )
#define FT_LOAD_CROP_BITMAP                  ( 1L << 6 )
#define FT_LOAD_PEDANTIC                     ( 1L << 7 )
#define FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH  ( 1L << 9 )
#define FT_LOAD_NO_RECURSE                   ( 1L << 10 )
#define FT_LOAD_IGNORE_TRANSFORM             ( 1L << 11 )
#define FT_LOAD_MONOCHROME                   ( 1L << 12 )
#define FT_LOAD_LINEAR_DESIGN                ( 1L << 13 )
#define FT_LOAD_NO_AUTOHINT                  ( 1L << 15 )

  /* */

  /* used internally only by certain font drivers! */
#define FT_LOAD_ADVANCE_ONLY                 ( 1L << 8 )
#define FT_LOAD_SBITS_ONLY                   ( 1L << 14 )

  /**************************************************************************
   *
   * @enum:
   *   FT_LOAD_TARGET_XXX
   *
   * @description:
   *   A list of values that are used to select a specific hinting algorithm
   *   to use by the hinter.  You should OR one of these values to your
   *   `load_flags' when calling @FT_Load_Glyph.
   *
   *   Note that font's native hinters may ignore the hinting algorithm you
   *   have specified (e.g., the TrueType bytecode interpreter).  You can set
   *   @FT_LOAD_FORCE_AUTOHINT to ensure that the auto-hinter is used.
   *
   *   Also note that @FT_LOAD_TARGET_LIGHT is an exception, in that it
   *   always implies @FT_LOAD_FORCE_AUTOHINT.
   *
   * @values:
   *   FT_LOAD_TARGET_NORMAL ::
   *     This corresponds to the default hinting algorithm, optimized for
   *     standard gray-level rendering.  For monochrome output, use
   *     @FT_LOAD_TARGET_MONO instead.
   *
   *   FT_LOAD_TARGET_LIGHT ::
   *     A lighter hinting algorithm for non-monochrome modes.  Many
   *     generated glyphs are more fuzzy but better resemble its original
   *     shape.  A bit like rendering on Mac OS~X.
   *
   *     As a special exception, this target implies @FT_LOAD_FORCE_AUTOHINT.
   *
   *   FT_LOAD_TARGET_MONO ::
   *     Strong hinting algorithm that should only be used for monochrome
   *     output.  The result is probably unpleasant if the glyph is rendered
   *     in non-monochrome modes.
   *
   *   FT_LOAD_TARGET_LCD ::
   *     A variant of @FT_LOAD_TARGET_NORMAL optimized for horizontally
   *     decimated LCD displays.
   *
   *   FT_LOAD_TARGET_LCD_V ::
   *     A variant of @FT_LOAD_TARGET_NORMAL optimized for vertically
   *     decimated LCD displays.
   *
   * @note:
   *   You should use only _one_ of the FT_LOAD_TARGET_XXX values in your
   *   `load_flags'.  They can't be ORed.
   *
   *   If @FT_LOAD_RENDER is also set, the glyph is rendered in the
   *   corresponding mode (i.e., the mode which matches the used algorithm
   *   best).  An exeption is FT_LOAD_TARGET_MONO since it implies
   *   @FT_LOAD_MONOCHROME.
   *
   *   You can use a hinting algorithm that doesn't correspond to the same
   *   rendering mode.  As an example, it is possible to use the `light'
   *   hinting algorithm and have the results rendered in horizontal LCD
   *   pixel mode, with code like
   *
   *     {
   *       FT_Load_Glyph( face, glyph_index,
   *                      load_flags | FT_LOAD_TARGET_LIGHT );
   *
   *       FT_Render_Glyph( face->glyph, FT_RENDER_MODE_LCD );
   *     }
   *
   */
#define FT_LOAD_TARGET_( x )   ( (FT_Int32)( (x) & 15 ) << 16 )

#define FT_LOAD_TARGET_NORMAL  FT_LOAD_TARGET_( FT_RENDER_MODE_NORMAL )
#define FT_LOAD_TARGET_LIGHT   FT_LOAD_TARGET_( FT_RENDER_MODE_LIGHT  )
#define FT_LOAD_TARGET_MONO    FT_LOAD_TARGET_( FT_RENDER_MODE_MONO   )
#define FT_LOAD_TARGET_LCD     FT_LOAD_TARGET_( FT_RENDER_MODE_LCD    )
#define FT_LOAD_TARGET_LCD_V   FT_LOAD_TARGET_( FT_RENDER_MODE_LCD_V  )

  /**************************************************************************
   *
   * @macro:
   *   FT_LOAD_TARGET_MODE
   *
   * @description:
   *   Return the @FT_Render_Mode corresponding to a given
   *   @FT_LOAD_TARGET_XXX value.
   *
   */
#define FT_LOAD_TARGET_MODE( x )  ( (FT_Render_Mode)( ( (x) >> 16 ) & 15 ) )

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Transform                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to set the transformation that is applied to glyph */
  /*    images when they are loaded into a glyph slot through              */
  /*    @FT_Load_Glyph.                                                    */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face   :: A handle to the source face object.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    matrix :: A pointer to the transformation's 2x2 matrix.  Use~0 for */
  /*              the identity matrix.                                     */
  /*    delta  :: A pointer to the translation vector.  Use~0 for the null */
  /*              vector.                                                  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The transformation is only applied to scalable image formats after */
  /*    the glyph has been loaded.  It means that hinting is unaltered by  */
  /*    the transformation and is performed on the character size given in */
  /*    the last call to @FT_Set_Char_Size or @FT_Set_Pixel_Sizes.         */
  /*                                                                       */
  /*    Note that this also transforms the `face.glyph.advance' field, but */
  /*    *not* the values in `face.glyph.metrics'.                          */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Set_Transform( FT_Face     face,
					FT_Matrix*  matrix,
					FT_Vector*  delta );

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Render_Mode                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration type that lists the render modes supported by       */
  /*    FreeType~2.  Each mode corresponds to a specific type of scanline  */
  /*    conversion performed on the outline.                               */
  /*                                                                       */
  /*    For bitmap fonts and embedded bitmaps the `bitmap->pixel_mode'     */
  /*    field in the @FT_GlyphSlotRec structure gives the format of the    */
  /*    returned bitmap.                                                   */
  /*                                                                       */
  /*    All modes except @FT_RENDER_MODE_MONO use 256 levels of opacity.   */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_RENDER_MODE_NORMAL ::                                           */
  /*      This is the default render mode; it corresponds to 8-bit         */
  /*      anti-aliased bitmaps.                                            */
  /*                                                                       */
  /*    FT_RENDER_MODE_LIGHT ::                                            */
  /*      This is equivalent to @FT_RENDER_MODE_NORMAL.  It is only        */
  /*      defined as a separate value because render modes are also used   */
  /*      indirectly to define hinting algorithm selectors.  See           */
  /*      @FT_LOAD_TARGET_XXX for details.                                 */
  /*                                                                       */
  /*    FT_RENDER_MODE_MONO ::                                             */
  /*      This mode corresponds to 1-bit bitmaps (with 2~levels of         */
  /*      opacity).                                                        */
  /*                                                                       */
  /*    FT_RENDER_MODE_LCD ::                                              */
  /*      This mode corresponds to horizontal RGB and BGR sub-pixel        */
  /*      displays like LCD screens.  It produces 8-bit bitmaps that are   */
  /*      3~times the width of the original glyph outline in pixels, and   */
  /*      which use the @FT_PIXEL_MODE_LCD mode.                           */
  /*                                                                       */
  /*    FT_RENDER_MODE_LCD_V ::                                            */
  /*      This mode corresponds to vertical RGB and BGR sub-pixel displays */
  /*      (like PDA screens, rotated LCD displays, etc.).  It produces     */
  /*      8-bit bitmaps that are 3~times the height of the original        */
  /*      glyph outline in pixels and use the @FT_PIXEL_MODE_LCD_V mode.   */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The LCD-optimized glyph bitmaps produced by FT_Render_Glyph can be */
  /*    filtered to reduce color-fringes by using @FT_Library_SetLcdFilter */
  /*    (not active in the default builds).  It is up to the caller to     */
  /*    either call @FT_Library_SetLcdFilter (if available) or do the      */
  /*    filtering itself.                                                  */
  /*                                                                       */
  /*    The selected render mode only affects vector glyphs of a font.     */
  /*    Embedded bitmaps often have a different pixel mode like            */
  /*    @FT_PIXEL_MODE_MONO.  You can use @FT_Bitmap_Convert to transform  */
  /*    them into 8-bit pixmaps.                                           */
  /*                                                                       */
  typedef enum  FT_Render_Mode_
  {
	FT_RENDER_MODE_NORMAL = 0,
	FT_RENDER_MODE_LIGHT,
	FT_RENDER_MODE_MONO,
	FT_RENDER_MODE_LCD,
	FT_RENDER_MODE_LCD_V,

	FT_RENDER_MODE_MAX

  } FT_Render_Mode;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    ft_render_mode_xxx                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    These constants are deprecated.  Use the corresponding             */
  /*    @FT_Render_Mode values instead.                                    */
  /*                                                                       */
  /* <Values>                                                              */
  /*    ft_render_mode_normal :: see @FT_RENDER_MODE_NORMAL                */
  /*    ft_render_mode_mono   :: see @FT_RENDER_MODE_MONO                  */
  /*                                                                       */
#define ft_render_mode_normal  FT_RENDER_MODE_NORMAL
#define ft_render_mode_mono    FT_RENDER_MODE_MONO

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Render_Glyph                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Convert a given glyph image to a bitmap.  It does so by inspecting */
  /*    the glyph image format, finding the relevant renderer, and         */
  /*    invoking it.                                                       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    slot        :: A handle to the glyph slot containing the image to  */
  /*                   convert.                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    render_mode :: This is the render mode used to render the glyph    */
  /*                   image into a bitmap.  See @FT_Render_Mode for a     */
  /*                   list of possible values.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Render_Glyph( FT_GlyphSlot    slot,
				   FT_Render_Mode  render_mode );

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Kerning_Mode                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An enumeration used to specify which kerning values to return in   */
  /*    @FT_Get_Kerning.                                                   */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_KERNING_DEFAULT  :: Return scaled and grid-fitted kerning       */
  /*                           distances (value is~0).                     */
  /*                                                                       */
  /*    FT_KERNING_UNFITTED :: Return scaled but un-grid-fitted kerning    */
  /*                           distances.                                  */
  /*                                                                       */
  /*    FT_KERNING_UNSCALED :: Return the kerning vector in original font  */
  /*                           units.                                      */
  /*                                                                       */
  typedef enum  FT_Kerning_Mode_
  {
	FT_KERNING_DEFAULT  = 0,
	FT_KERNING_UNFITTED,
	FT_KERNING_UNSCALED

  } FT_Kerning_Mode;

  /*************************************************************************/
  /*                                                                       */
  /* <Const>                                                               */
  /*    ft_kerning_default                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This constant is deprecated.  Please use @FT_KERNING_DEFAULT       */
  /*    instead.                                                           */
  /*                                                                       */
#define ft_kerning_default   FT_KERNING_DEFAULT

  /*************************************************************************/
  /*                                                                       */
  /* <Const>                                                               */
  /*    ft_kerning_unfitted                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This constant is deprecated.  Please use @FT_KERNING_UNFITTED      */
  /*    instead.                                                           */
  /*                                                                       */
#define ft_kerning_unfitted  FT_KERNING_UNFITTED

  /*************************************************************************/
  /*                                                                       */
  /* <Const>                                                               */
  /*    ft_kerning_unscaled                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This constant is deprecated.  Please use @FT_KERNING_UNSCALED      */
  /*    instead.                                                           */
  /*                                                                       */
#define ft_kerning_unscaled  FT_KERNING_UNSCALED

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Kerning                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return the kerning vector between two glyphs of a same face.       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to a source face object.                   */
  /*                                                                       */
  /*    left_glyph  :: The index of the left glyph in the kern pair.       */
  /*                                                                       */
  /*    right_glyph :: The index of the right glyph in the kern pair.      */
  /*                                                                       */
  /*    kern_mode   :: See @FT_Kerning_Mode for more information.          */
  /*                   Determines the scale and dimension of the returned  */
  /*                   kerning vector.                                     */
  /*                                                                       */
  /* <Output>                                                              */
  /*    akerning    :: The kerning vector.  This is either in font units   */
  /*                   or in pixels (26.6 format) for scalable formats,    */
  /*                   and in pixels for fixed-sizes formats.              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Only horizontal layouts (left-to-right & right-to-left) are        */
  /*    supported by this method.  Other layouts, or more sophisticated    */
  /*    kernings, are out of the scope of this API function -- they can be */
  /*    implemented through format-specific interfaces.                    */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Get_Kerning( FT_Face     face,
				  FT_UInt     left_glyph,
				  FT_UInt     right_glyph,
				  FT_UInt     kern_mode,
				  FT_Vector  *akerning );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Track_Kerning                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return the track kerning for a given face object at a given size.  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face       :: A handle to a source face object.                    */
  /*                                                                       */
  /*    point_size :: The point size in 16.16 fractional points.           */
  /*                                                                       */
  /*    degree     :: The degree of tightness.  Increasingly negative      */
  /*                  values represent tighter track kerning, while        */
  /*                  increasingly positive values represent looser track  */
  /*                  kerning.  Value zero means no track kerning.         */
  /*                                                                       */
  /* <Output>                                                              */
  /*    akerning   :: The kerning in 16.16 fractional points, to be        */
  /*                  uniformly applied between all glyphs.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Currently, only the Type~1 font driver supports track kerning,     */
  /*    using data from AFM files (if attached with @FT_Attach_File or     */
  /*    @FT_Attach_Stream).                                                */
  /*                                                                       */
  /*    Only very few AFM files come with track kerning data; please refer */
  /*    to the Adobe's AFM specification for more details.                 */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Get_Track_Kerning( FT_Face    face,
						FT_Fixed   point_size,
						FT_Int     degree,
						FT_Fixed*  akerning );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Glyph_Name                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieve the ASCII name of a given glyph in a face.  This only     */
  /*    works for those faces where @FT_HAS_GLYPH_NAMES(face) returns~1.   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face        :: A handle to a source face object.                   */
  /*                                                                       */
  /*    glyph_index :: The glyph index.                                    */
  /*                                                                       */
  /*    buffer_max  :: The maximum number of bytes available in the        */
  /*                   buffer.                                             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    buffer      :: A pointer to a target buffer where the name is      */
  /*                   copied to.                                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    An error is returned if the face doesn't provide glyph names or if */
  /*    the glyph index is invalid.  In all cases of failure, the first    */
  /*    byte of `buffer' is set to~0 to indicate an empty name.            */
  /*                                                                       */
  /*    The glyph name is truncated to fit within the buffer if it is too  */
  /*    long.  The returned string is always zero-terminated.              */
  /*                                                                       */
  /*    Be aware that FreeType reorders glyph indices internally so that   */
  /*    glyph index~0 always corresponds to the `missing glyph' (called    */
  /*    `.notdef').                                                        */
  /*                                                                       */
  /*    This function is not compiled within the library if the config     */
  /*    macro `FT_CONFIG_OPTION_NO_GLYPH_NAMES' is defined in              */
  /*    `include/freetype/config/ftoptions.h'.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Get_Glyph_Name( FT_Face     face,
					 FT_UInt     glyph_index,
					 FT_Pointer  buffer,
					 FT_UInt     buffer_max );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Postscript_Name                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieve the ASCII PostScript name of a given face, if available.  */
  /*    This only works with PostScript and TrueType fonts.                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the source face object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A pointer to the face's PostScript name.  NULL if unavailable.     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The returned pointer is owned by the face and is destroyed with    */
  /*    it.                                                                */
  /*                                                                       */
  FT_EXPORT( const char* )
  FT_Get_Postscript_Name( FT_Face  face );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Select_Charmap                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Select a given charmap by its encoding tag (as listed in           */
  /*    `freetype.h').                                                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face     :: A handle to the source face object.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    encoding :: A handle to the selected encoding.                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function returns an error if no charmap in the face           */
  /*    corresponds to the encoding queried here.                          */
  /*                                                                       */
  /*    Because many fonts contain more than a single cmap for Unicode     */
  /*    encoding, this function has some special code to select the one    */
  /*    which covers Unicode best (`best' in the sense that a UCS-4 cmap   */
  /*    is preferred to a UCS-2 cmap).  It is thus preferable to           */
  /*    @FT_Set_Charmap in this case.                                      */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Select_Charmap( FT_Face      face,
					 FT_Encoding  encoding );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Charmap                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Select a given charmap for character code to glyph index mapping.  */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face    :: A handle to the source face object.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    charmap :: A handle to the selected charmap.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function returns an error if the charmap is not part of       */
  /*    the face (i.e., if it is not listed in the `face->charmaps'        */
  /*    table).                                                            */
  /*                                                                       */
  /*    It also fails if a type~14 charmap is selected.                    */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Set_Charmap( FT_Face     face,
				  FT_CharMap  charmap );

  /*************************************************************************
   *
   * @function:
   *   FT_Get_Charmap_Index
   *
   * @description:
   *   Retrieve index of a given charmap.
   *
   * @input:
   *   charmap ::
   *     A handle to a charmap.
   *
   * @return:
   *   The index into the array of character maps within the face to which
   *   `charmap' belongs.  If an error occurs, -1 is returned.
   *
   */
  FT_EXPORT( FT_Int )
  FT_Get_Charmap_Index( FT_CharMap  charmap );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Char_Index                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return the glyph index of a given character code.  This function   */
  /*    uses a charmap object to do the mapping.                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face     :: A handle to the source face object.                    */
  /*                                                                       */
  /*    charcode :: The character code.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The glyph index.  0~means `undefined character code'.              */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If you use FreeType to manipulate the contents of font files       */
  /*    directly, be aware that the glyph index returned by this function  */
  /*    doesn't always correspond to the internal indices used within the  */
  /*    file.  This is done to ensure that value~0 always corresponds to   */
  /*    the `missing glyph'.  If the first glyph is not named `.notdef',   */
  /*    then for Type~1 and Type~42 fonts, `.notdef' will be moved into    */
  /*    the glyph ID~0 position, and whatever was there will be moved to   */
  /*    the position `.notdef' had.  For Type~1 fonts, if there is no      */
  /*    `.notdef' glyph at all, then one will be created at index~0 and    */
  /*    whatever was there will be moved to the last index -- Type~42      */
  /*    fonts are considered invalid under this condition.                 */
  /*                                                                       */
  FT_EXPORT( FT_UInt )
  FT_Get_Char_Index( FT_Face   face,
					 FT_ULong  charcode );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_First_Char                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used to return the first character code in the    */
  /*    current charmap of a given face.  It also returns the              */
  /*    corresponding glyph index.                                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face    :: A handle to the source face object.                     */
  /*                                                                       */
  /* <Output>                                                              */
  /*    agindex :: Glyph index of first character code.  0~if charmap is   */
  /*               empty.                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The charmap's first character code.                                */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You should use this function with @FT_Get_Next_Char to be able to  */
  /*    parse all character codes available in a given charmap.  The code  */
  /*    should look like this:                                             */
  /*                                                                       */
  /*    {                                                                  */
  /*      FT_ULong  charcode;                                              */
  /*      FT_UInt   gindex;                                                */
  /*                                                                       */
  /*                                                                       */
  /*      charcode = FT_Get_First_Char( face, &gindex );                   */
  /*      while ( gindex != 0 )                                            */
  /*      {                                                                */
  /*        ... do something with (charcode,gindex) pair ...               */
  /*                                                                       */
  /*        charcode = FT_Get_Next_Char( face, charcode, &gindex );        */
  /*      }                                                                */
  /*    }                                                                  */
  /*                                                                       */
  /*    Note that `*agindex' is set to~0 if the charmap is empty.  The     */
  /*    result itself can be~0 in two cases: if the charmap is empty or    */
  /*    if the value~0 is the first valid character code.                  */
  /*                                                                       */
  FT_EXPORT( FT_ULong )
  FT_Get_First_Char( FT_Face   face,
					 FT_UInt  *agindex );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Next_Char                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This function is used to return the next character code in the     */
  /*    current charmap of a given face following the value `char_code',   */
  /*    as well as the corresponding glyph index.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face      :: A handle to the source face object.                   */
  /*    char_code :: The starting character code.                          */
  /*                                                                       */
  /* <Output>                                                              */
  /*    agindex   :: Glyph index of next character code.  0~if charmap     */
  /*                 is empty.                                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The charmap's next character code.                                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You should use this function with @FT_Get_First_Char to walk       */
  /*    over all character codes available in a given charmap.  See the    */
  /*    note for this function for a simple code example.                  */
  /*                                                                       */
  /*    Note that `*agindex' is set to~0 when there are no more codes in   */
  /*    the charmap.                                                       */
  /*                                                                       */
  FT_EXPORT( FT_ULong )
  FT_Get_Next_Char( FT_Face    face,
					FT_ULong   char_code,
					FT_UInt   *agindex );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Name_Index                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return the glyph index of a given glyph name.  This function uses  */
  /*    driver specific objects to do the translation.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face       :: A handle to the source face object.                  */
  /*                                                                       */
  /*    glyph_name :: The glyph name.                                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The glyph index.  0~means `undefined character code'.              */
  /*                                                                       */
  FT_EXPORT( FT_UInt )
  FT_Get_Name_Index( FT_Face     face,
					 FT_String*  glyph_name );

  /*************************************************************************
   *
   * @macro:
   *   FT_SUBGLYPH_FLAG_XXX
   *
   * @description:
   *   A list of constants used to describe subglyphs.  Please refer to the
   *   TrueType specification for the meaning of the various flags.
   *
   * @values:
   *   FT_SUBGLYPH_FLAG_ARGS_ARE_WORDS ::
   *   FT_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES ::
   *   FT_SUBGLYPH_FLAG_ROUND_XY_TO_GRID ::
   *   FT_SUBGLYPH_FLAG_SCALE ::
   *   FT_SUBGLYPH_FLAG_XY_SCALE ::
   *   FT_SUBGLYPH_FLAG_2X2 ::
   *   FT_SUBGLYPH_FLAG_USE_MY_METRICS ::
   *
   */
#define FT_SUBGLYPH_FLAG_ARGS_ARE_WORDS          1
#define FT_SUBGLYPH_FLAG_ARGS_ARE_XY_VALUES      2
#define FT_SUBGLYPH_FLAG_ROUND_XY_TO_GRID        4
#define FT_SUBGLYPH_FLAG_SCALE                   8
#define FT_SUBGLYPH_FLAG_XY_SCALE             0x40
#define FT_SUBGLYPH_FLAG_2X2                  0x80
#define FT_SUBGLYPH_FLAG_USE_MY_METRICS      0x200

  /*************************************************************************
   *
   * @func:
   *   FT_Get_SubGlyph_Info
   *
   * @description:
   *   Retrieve a description of a given subglyph.  Only use it if
   *   `glyph->format' is @FT_GLYPH_FORMAT_COMPOSITE; an error is
   *   returned otherwise.
   *
   * @input:
   *   glyph ::
   *     The source glyph slot.
   *
   *   sub_index ::
   *     The index of the subglyph.  Must be less than
   *     `glyph->num_subglyphs'.
   *
   * @output:
   *   p_index ::
   *     The glyph index of the subglyph.
   *
   *   p_flags ::
   *     The subglyph flags, see @FT_SUBGLYPH_FLAG_XXX.
   *
   *   p_arg1 ::
   *     The subglyph's first argument (if any).
   *
   *   p_arg2 ::
   *     The subglyph's second argument (if any).
   *
   *   p_transform ::
   *     The subglyph transformation (if any).
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   The values of `*p_arg1', `*p_arg2', and `*p_transform' must be
   *   interpreted depending on the flags returned in `*p_flags'.  See the
   *   TrueType specification for details.
   *
   */
  FT_EXPORT( FT_Error )
  FT_Get_SubGlyph_Info( FT_GlyphSlot  glyph,
						FT_UInt       sub_index,
						FT_Int       *p_index,
						FT_UInt      *p_flags,
						FT_Int       *p_arg1,
						FT_Int       *p_arg2,
						FT_Matrix    *p_transform );

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_FSTYPE_XXX                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A list of bit flags used in the `fsType' field of the OS/2 table   */
  /*    in a TrueType or OpenType font and the `FSType' entry in a         */
  /*    PostScript font.  These bit flags are returned by                  */
  /*    @FT_Get_FSType_Flags; they inform client applications of embedding */
  /*    and subsetting restrictions associated with a font.                */
  /*                                                                       */
  /*    See http://www.adobe.com/devnet/acrobat/pdfs/FontPolicies.pdf for  */
  /*    more details.                                                      */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_FSTYPE_INSTALLABLE_EMBEDDING ::                                 */
  /*      Fonts with no fsType bit set may be embedded and permanently     */
  /*      installed on the remote system by an application.                */
  /*                                                                       */
  /*    FT_FSTYPE_RESTRICTED_LICENSE_EMBEDDING ::                          */
  /*      Fonts that have only this bit set must not be modified, embedded */
  /*      or exchanged in any manner without first obtaining permission of */
  /*      the font software copyright owner.                               */
  /*                                                                       */
  /*    FT_FSTYPE_PREVIEW_AND_PRINT_EMBEDDING ::                           */
  /*      If this bit is set, the font may be embedded and temporarily     */
  /*      loaded on the remote system.  Documents containing Preview &     */
  /*      Print fonts must be opened `read-only'; no edits can be applied  */
  /*      to the document.                                                 */
  /*                                                                       */
  /*    FT_FSTYPE_EDITABLE_EMBEDDING ::                                    */
  /*      If this bit is set, the font may be embedded but must only be    */
  /*      installed temporarily on other systems.  In contrast to Preview  */
  /*      & Print fonts, documents containing editable fonts may be opened */
  /*      for reading, editing is permitted, and changes may be saved.     */
  /*                                                                       */
  /*    FT_FSTYPE_NO_SUBSETTING ::                                         */
  /*      If this bit is set, the font may not be subsetted prior to       */
  /*      embedding.                                                       */
  /*                                                                       */
  /*    FT_FSTYPE_BITMAP_EMBEDDING_ONLY ::                                 */
  /*      If this bit is set, only bitmaps contained in the font may be    */
  /*      embedded; no outline data may be embedded.  If there are no      */
  /*      bitmaps available in the font, then the font is unembeddable.    */
  /*                                                                       */
  /* <Note>                                                                */
  /*    While the fsType flags can indicate that a font may be embedded, a */
  /*    license with the font vendor may be separately required to use the */
  /*    font in this way.                                                  */
  /*                                                                       */
#define FT_FSTYPE_INSTALLABLE_EMBEDDING         0x0000
#define FT_FSTYPE_RESTRICTED_LICENSE_EMBEDDING  0x0002
#define FT_FSTYPE_PREVIEW_AND_PRINT_EMBEDDING   0x0004
#define FT_FSTYPE_EDITABLE_EMBEDDING            0x0008
#define FT_FSTYPE_NO_SUBSETTING                 0x0100
#define FT_FSTYPE_BITMAP_EMBEDDING_ONLY         0x0200

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_FSType_Flags                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return the fsType flags for a font.                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A handle to the source face object.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The fsType flags, @FT_FSTYPE_XXX.                                  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Use this function rather than directly reading the `fs_type' field */
  /*    in the @PS_FontInfoRec structure which is only guaranteed to       */
  /*    return the correct results for Type~1 fonts.                       */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.3.8                                                              */
  /*                                                                       */
  FT_EXPORT( FT_UShort )
  FT_Get_FSType_Flags( FT_Face  face );

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    glyph_variants                                                     */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Glyph Variants                                                     */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    The FreeType~2 interface to Unicode Ideographic Variation          */
  /*    Sequences (IVS), using the SFNT cmap format~14.                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Many CJK characters have variant forms.  They are a sort of grey   */
  /*    area somewhere between being totally irrelevant and semantically   */
  /*    distinct; for this reason, the Unicode consortium decided to       */
  /*    introduce Ideographic Variation Sequences (IVS), consisting of a   */
  /*    Unicode base character and one of 240 variant selectors            */
  /*    (U+E0100-U+E01EF), instead of further extending the already huge   */
  /*    code range for CJK characters.                                     */
  /*                                                                       */
  /*    An IVS is registered and unique; for further details please refer  */
  /*    to Unicode Technical Report #37, the Ideographic Variation         */
  /*    Database.  To date (October 2007), the character with the most     */
  /*    variants is U+908A, having 8~such IVS.                             */
  /*                                                                       */
  /*    Adobe and MS decided to support IVS with a new cmap subtable       */
  /*    (format~14).  It is an odd subtable because it is not a mapping of */
  /*    input code points to glyphs, but contains lists of all variants    */
  /*    supported by the font.                                             */
  /*                                                                       */
  /*    A variant may be either `default' or `non-default'.  A default     */
  /*    variant is the one you will get for that code point if you look it */
  /*    up in the standard Unicode cmap.  A non-default variant is a       */
  /*    different glyph.                                                   */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Face_GetCharVariantIndex                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return the glyph index of a given character code as modified by    */
  /*    the variation selector.                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face ::                                                            */
  /*      A handle to the source face object.                              */
  /*                                                                       */
  /*    charcode ::                                                        */
  /*      The character code point in Unicode.                             */
  /*                                                                       */
  /*    variantSelector ::                                                 */
  /*      The Unicode code point of the variation selector.                */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The glyph index.  0~means either `undefined character code', or    */
  /*    `undefined selector code', or `no variation selector cmap          */
  /*    subtable', or `current CharMap is not Unicode'.                    */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If you use FreeType to manipulate the contents of font files       */
  /*    directly, be aware that the glyph index returned by this function  */
  /*    doesn't always correspond to the internal indices used within      */
  /*    the file.  This is done to ensure that value~0 always corresponds  */
  /*    to the `missing glyph'.                                            */
  /*                                                                       */
  /*    This function is only meaningful if                                */
  /*      a) the font has a variation selector cmap sub table,             */
  /*    and                                                                */
  /*      b) the current charmap has a Unicode encoding.                   */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.3.6                                                              */
  /*                                                                       */
  FT_EXPORT( FT_UInt )
  FT_Face_GetCharVariantIndex( FT_Face   face,
							   FT_ULong  charcode,
							   FT_ULong  variantSelector );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Face_GetCharVariantIsDefault                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Check whether this variant of this Unicode character is the one to */
  /*    be found in the `cmap'.                                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face ::                                                            */
  /*      A handle to the source face object.                              */
  /*                                                                       */
  /*    charcode ::                                                        */
  /*      The character codepoint in Unicode.                              */
  /*                                                                       */
  /*    variantSelector ::                                                 */
  /*      The Unicode codepoint of the variation selector.                 */
  /*                                                                       */
  /* <Return>                                                              */
  /*    1~if found in the standard (Unicode) cmap, 0~if found in the       */
  /*    variation selector cmap, or -1 if it is not a variant.             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function is only meaningful if the font has a variation       */
  /*    selector cmap subtable.                                            */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.3.6                                                              */
  /*                                                                       */
  FT_EXPORT( FT_Int )
  FT_Face_GetCharVariantIsDefault( FT_Face   face,
								   FT_ULong  charcode,
								   FT_ULong  variantSelector );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Face_GetVariantSelectors                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return a zero-terminated list of Unicode variant selectors found   */
  /*    in the font.                                                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face ::                                                            */
  /*      A handle to the source face object.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A pointer to an array of selector code points, or NULL if there is */
  /*    no valid variant selector cmap subtable.                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The last item in the array is~0; the array is owned by the         */
  /*    @FT_Face object but can be overwritten or released on the next     */
  /*    call to a FreeType function.                                       */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.3.6                                                              */
  /*                                                                       */
  FT_EXPORT( FT_UInt32* )
  FT_Face_GetVariantSelectors( FT_Face  face );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Face_GetVariantsOfChar                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return a zero-terminated list of Unicode variant selectors found   */
  /*    for the specified character code.                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face ::                                                            */
  /*      A handle to the source face object.                              */
  /*                                                                       */
  /*    charcode ::                                                        */
  /*      The character codepoint in Unicode.                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A pointer to an array of variant selector code points which are    */
  /*    active for the given character, or NULL if the corresponding list  */
  /*    is empty.                                                          */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The last item in the array is~0; the array is owned by the         */
  /*    @FT_Face object but can be overwritten or released on the next     */
  /*    call to a FreeType function.                                       */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.3.6                                                              */
  /*                                                                       */
  FT_EXPORT( FT_UInt32* )
  FT_Face_GetVariantsOfChar( FT_Face   face,
							 FT_ULong  charcode );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Face_GetCharsOfVariant                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return a zero-terminated list of Unicode character codes found for */
  /*    the specified variant selector.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face ::                                                            */
  /*      A handle to the source face object.                              */
  /*                                                                       */
  /*    variantSelector ::                                                 */
  /*      The variant selector code point in Unicode.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A list of all the code points which are specified by this selector */
  /*    (both default and non-default codes are returned) or NULL if there */
  /*    is no valid cmap or the variant selector is invalid.               */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The last item in the array is~0; the array is owned by the         */
  /*    @FT_Face object but can be overwritten or released on the next     */
  /*    call to a FreeType function.                                       */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.3.6                                                              */
  /*                                                                       */
  FT_EXPORT( FT_UInt32* )
  FT_Face_GetCharsOfVariant( FT_Face   face,
							 FT_ULong  variantSelector );

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    computations                                                       */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Computations                                                       */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Crunching fixed numbers and vectors.                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains various functions used to perform            */
  /*    computations on 16.16 fixed-float numbers or 2d vectors.           */
  /*                                                                       */
  /* <Order>                                                               */
  /*    FT_MulDiv                                                          */
  /*    FT_MulFix                                                          */
  /*    FT_DivFix                                                          */
  /*    FT_RoundFix                                                        */
  /*    FT_CeilFix                                                         */
  /*    FT_FloorFix                                                        */
  /*    FT_Vector_Transform                                                */
  /*    FT_Matrix_Multiply                                                 */
  /*    FT_Matrix_Invert                                                   */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulDiv                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation `(a*b)/c'   */
  /*    with maximum accuracy (it uses a 64-bit intermediate integer       */
  /*    whenever necessary).                                               */
  /*                                                                       */
  /*    This function isn't necessarily as fast as some processor specific */
  /*    operations, but is at least completely portable.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.                                        */
  /*    c :: The divisor.                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*b)/c'.  This function never traps when trying to */
  /*    divide by zero; it simply returns `MaxInt' or `MinInt' depending   */
  /*    on the signs of `a' and `b'.                                       */
  /*                                                                       */
  FT_EXPORT( FT_Long )
  FT_MulDiv( FT_Long  a,
			 FT_Long  b,
			 FT_Long  c );

  /* */

  /* The following #if 0 ... #endif is for the documentation formatter, */
  /* hiding the internal `FT_MULFIX_INLINED' macro.                     */

#if 0
  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_MulFix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation             */
  /*    `(a*b)/0x10000' with maximum accuracy.  Most of the time this is   */
  /*    used to multiply a given value by a 16.16 fixed float factor.      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.  Use a 16.16 factor here whenever      */
  /*         possible (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*b)/0x10000'.                                     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function has been optimized for the case where the absolute   */
  /*    value of `a' is less than 2048, and `b' is a 16.16 scaling factor. */
  /*    As this happens mainly when scaling from notional units to         */
  /*    fractional pixels in FreeType, it resulted in noticeable speed     */
  /*    improvements between versions 2.x and 1.x.                         */
  /*                                                                       */
  /*    As a conclusion, always try to place a 16.16 factor as the         */
  /*    _second_ argument of this function; this can make a great          */
  /*    difference.                                                        */
  /*                                                                       */
  FT_EXPORT( FT_Long )
  FT_MulFix( FT_Long  a,
			 FT_Long  b );

  /* */
#endif

#ifdef FT_MULFIX_INLINED
#define FT_MulFix( a, b )  FT_MULFIX_INLINED( a, b )
#else
  FT_EXPORT( FT_Long )
  FT_MulFix( FT_Long  a,
			 FT_Long  b );
#endif

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_DivFix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to perform the computation             */
  /*    `(a*0x10000)/b' with maximum accuracy.  Most of the time, this is  */
  /*    used to divide a given value by a 16.16 fixed float factor.        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The first multiplier.                                         */
  /*    b :: The second multiplier.  Use a 16.16 factor here whenever      */
  /*         possible (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a*0x10000)/b'.                                     */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The optimization for FT_DivFix() is simple: If (a~<<~16) fits in   */
  /*    32~bits, then the division is computed directly.  Otherwise, we    */
  /*    use a specialized version of @FT_MulDiv.                           */
  /*                                                                       */
  FT_EXPORT( FT_Long )
  FT_DivFix( FT_Long  a,
			 FT_Long  b );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_RoundFix                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to round a 16.16 fixed number.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The number to be rounded.                                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a + 0x8000) & -0x10000'.                           */
  /*                                                                       */
  FT_EXPORT( FT_Fixed )
  FT_RoundFix( FT_Fixed  a );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_CeilFix                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to compute the ceiling function of a   */
  /*    16.16 fixed number.                                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The number for which the ceiling function is to be computed.  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `(a + 0x10000 - 1) & -0x10000'.                      */
  /*                                                                       */
  FT_EXPORT( FT_Fixed )
  FT_CeilFix( FT_Fixed  a );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_FloorFix                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very simple function used to compute the floor function of a     */
  /*    16.16 fixed number.                                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: The number for which the floor function is to be computed.    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The result of `a & -0x10000'.                                      */
  /*                                                                       */
  FT_EXPORT( FT_Fixed )
  FT_FloorFix( FT_Fixed  a );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Vector_Transform                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Transform a single vector through a 2x2 matrix.                    */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    vector :: The target vector to transform.                          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    matrix :: A pointer to the source 2x2 matrix.                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The result is undefined if either `vector' or `matrix' is invalid. */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Vector_Transform( FT_Vector*        vec,
					   const FT_Matrix*  matrix );

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    version                                                            */
  /*                                                                       */
  /* <Title>                                                               */
  /*    FreeType Version                                                   */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Functions and macros related to FreeType versions.                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Note that those functions and macros are of limited use because    */
  /*    even a new release of FreeType with only documentation changes     */
  /*    increases the version number.                                      */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************
   *
   * @enum:
   *   FREETYPE_XXX
   *
   * @description:
   *   These three macros identify the FreeType source code version.
   *   Use @FT_Library_Version to access them at runtime.
   *
   * @values:
   *   FREETYPE_MAJOR :: The major version number.
   *   FREETYPE_MINOR :: The minor version number.
   *   FREETYPE_PATCH :: The patch level.
   *
   * @note:
   *   The version number of FreeType if built as a dynamic link library
   *   with the `libtool' package is _not_ controlled by these three
   *   macros.
   *
   */
#define FREETYPE_MAJOR  2
#define FREETYPE_MINOR  4
#define FREETYPE_PATCH  10

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Library_Version                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return the version of the FreeType library being used.  This is    */
  /*    useful when dynamically linking to the library, since one cannot   */
  /*    use the macros @FREETYPE_MAJOR, @FREETYPE_MINOR, and               */
  /*    @FREETYPE_PATCH.                                                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A source library handle.                                */
  /*                                                                       */
  /* <Output>                                                              */
  /*    amajor  :: The major version number.                               */
  /*                                                                       */
  /*    aminor  :: The minor version number.                               */
  /*                                                                       */
  /*    apatch  :: The patch version number.                               */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The reason why this function takes a `library' argument is because */
  /*    certain programs implement library initialization in a custom way  */
  /*    that doesn't use @FT_Init_FreeType.                                */
  /*                                                                       */
  /*    In such cases, the library version might not be available before   */
  /*    the library object has been created.                               */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Library_Version( FT_Library   library,
					  FT_Int      *amajor,
					  FT_Int      *aminor,
					  FT_Int      *apatch );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Face_CheckTrueTypePatents                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Parse all bytecode instructions of a TrueType font file to check   */
  /*    whether any of the patented opcodes are used.  This is only useful */
  /*    if you want to be able to use the unpatented hinter with           */
  /*    fonts that do *not* use these opcodes.                             */
  /*                                                                       */
  /*    Note that this function parses *all* glyph instructions in the     */
  /*    font file, which may be slow.                                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face :: A face handle.                                             */
  /*                                                                       */
  /* <Return>                                                              */
  /*    1~if this is a TrueType font that uses one of the patented         */
  /*    opcodes, 0~otherwise.                                              */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Since May 2010, TrueType hinting is no longer patented.            */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.3.5                                                              */
  /*                                                                       */
  FT_EXPORT( FT_Bool )
  FT_Face_CheckTrueTypePatents( FT_Face  face );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Face_SetUnpatentedHinting                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Enable or disable the unpatented hinter for a given face.          */
  /*    Only enable it if you have determined that the face doesn't        */
  /*    use any patented opcodes (see @FT_Face_CheckTrueTypePatents).      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face  :: A face handle.                                            */
  /*                                                                       */
  /*    value :: New boolean setting.                                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    The old setting value.  This will always be false if this is not   */
  /*    an SFNT font, or if the unpatented hinter is not compiled in this  */
  /*    instance of the library.                                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Since May 2010, TrueType hinting is no longer patented.            */
  /*                                                                       */
  /* <Since>                                                               */
  /*    2.3.5                                                              */
  /*                                                                       */
  FT_EXPORT( FT_Bool )
  FT_Face_SetUnpatentedHinting( FT_Face  face,
								FT_Bool  value );

  /* */

FT_END_HEADER

#endif /* __FREETYPE_H__ */

/* END */

/*** End of inlined file: freetype.h ***/

/*
 * These headers are normally included manually but we add them
 * here for completeness.
 *
 */

/*** Start of inlined file: ftgasp.h ***/
/*                                                                         */
/*  ftgasp.h                                                               */
/*                                                                         */
/*    Access of TrueType's `gasp' table (specification).                   */
/*                                                                         */
/*  Copyright 2007, 2008, 2011 by                                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef _FT_GASP_H_
#define _FT_GASP_H_

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif

  /***************************************************************************
   *
   * @section:
   *   gasp_table
   *
   * @title:
   *   Gasp Table
   *
   * @abstract:
   *   Retrieving TrueType `gasp' table entries.
   *
   * @description:
   *   The function @FT_Get_Gasp can be used to query a TrueType or OpenType
   *   font for specific entries in its `gasp' table, if any.  This is
   *   mainly useful when implementing native TrueType hinting with the
   *   bytecode interpreter to duplicate the Windows text rendering results.
   */

  /*************************************************************************
   *
   * @enum:
   *   FT_GASP_XXX
   *
   * @description:
   *   A list of values and/or bit-flags returned by the @FT_Get_Gasp
   *   function.
   *
   * @values:
   *   FT_GASP_NO_TABLE ::
   *     This special value means that there is no GASP table in this face.
   *     It is up to the client to decide what to do.
   *
   *   FT_GASP_DO_GRIDFIT ::
   *     Grid-fitting and hinting should be performed at the specified ppem.
   *     This *really* means TrueType bytecode interpretation.  If this bit
   *     is not set, no hinting gets applied.
   *
   *   FT_GASP_DO_GRAY ::
   *     Anti-aliased rendering should be performed at the specified ppem.
   *     If not set, do monochrome rendering.
   *
   *   FT_GASP_SYMMETRIC_SMOOTHING ::
   *     If set, smoothing along multiple axes must be used with ClearType.
   *
   *   FT_GASP_SYMMETRIC_GRIDFIT ::
   *     Grid-fitting must be used with ClearType's symmetric smoothing.
   *
   * @note:
   *   The bit-flags `FT_GASP_DO_GRIDFIT' and `FT_GASP_DO_GRAY' are to be
   *   used for standard font rasterization only.  Independently of that,
   *   `FT_GASP_SYMMETRIC_SMOOTHING' and `FT_GASP_SYMMETRIC_GRIDFIT' are to
   *   be used if ClearType is enabled (and `FT_GASP_DO_GRIDFIT' and
   *   `FT_GASP_DO_GRAY' are consequently ignored).
   *
   *   `ClearType' is Microsoft's implementation of LCD rendering, partly
   *   protected by patents.
   *
   * @since:
   *   2.3.0
   */
#define FT_GASP_NO_TABLE               -1
#define FT_GASP_DO_GRIDFIT           0x01
#define FT_GASP_DO_GRAY              0x02
#define FT_GASP_SYMMETRIC_SMOOTHING  0x08
#define FT_GASP_SYMMETRIC_GRIDFIT    0x10

  /*************************************************************************
   *
   * @func:
   *   FT_Get_Gasp
   *
   * @description:
   *   Read the `gasp' table from a TrueType or OpenType font file and
   *   return the entry corresponding to a given character pixel size.
   *
   * @input:
   *   face :: The source face handle.
   *   ppem :: The vertical character pixel size.
   *
   * @return:
   *   Bit flags (see @FT_GASP_XXX), or @FT_GASP_NO_TABLE if there is no
   *   `gasp' table in the face.
   *
   * @since:
   *   2.3.0
   */
  FT_EXPORT( FT_Int )
  FT_Get_Gasp( FT_Face  face,
			   FT_UInt  ppem );

/* */

#endif /* _FT_GASP_H_ */

/* END */

/*** End of inlined file: ftgasp.h ***/



/*** Start of inlined file: ftglyph.h ***/
/*                                                                         */
/*  ftglyph.h                                                              */
/*                                                                         */
/*    FreeType convenience functions to handle glyphs (specification).     */
/*                                                                         */
/*  Copyright 1996-2003, 2006, 2008, 2009, 2011 by                         */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* This file contains the definition of several convenience functions    */
  /* that can be used by client applications to easily retrieve glyph      */
  /* bitmaps and outlines from a given face.                               */
  /*                                                                       */
  /* These functions should be optional if you are writing a font server   */
  /* or text layout engine on top of FreeType.  However, they are pretty   */
  /* handy for many other simple uses of the library.                      */
  /*                                                                       */
  /*************************************************************************/

#ifndef __FTGLYPH_H__
#define __FTGLYPH_H__

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    glyph_management                                                   */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Glyph Management                                                   */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Generic interface to manage individual glyph data.                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains definitions used to manage glyph data        */
  /*    through generic FT_Glyph objects.  Each of them can contain a      */
  /*    bitmap, a vector outline, or even images in other formats.         */
  /*                                                                       */
  /*************************************************************************/

  /* forward declaration to a private type */
  typedef struct FT_Glyph_Class_  FT_Glyph_Class;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Glyph                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Handle to an object used to model generic glyph images.  It is a   */
  /*    pointer to the @FT_GlyphRec structure and can contain a glyph      */
  /*    bitmap or pointer.                                                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Glyph objects are not owned by the library.  You must thus release */
  /*    them manually (through @FT_Done_Glyph) _before_ calling            */
  /*    @FT_Done_FreeType.                                                 */
  /*                                                                       */
  typedef struct FT_GlyphRec_*  FT_Glyph;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_GlyphRec                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The root glyph structure contains a given glyph image plus its     */
  /*    advance width in 16.16 fixed float format.                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    library :: A handle to the FreeType library object.                */
  /*                                                                       */
  /*    clazz   :: A pointer to the glyph's class.  Private.               */
  /*                                                                       */
  /*    format  :: The format of the glyph's image.                        */
  /*                                                                       */
  /*    advance :: A 16.16 vector that gives the glyph's advance width.    */
  /*                                                                       */
  typedef struct  FT_GlyphRec_
  {
	FT_Library             library;
	const FT_Glyph_Class*  clazz;
	FT_Glyph_Format        format;
	FT_Vector              advance;

  } FT_GlyphRec;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_BitmapGlyph                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an object used to model a bitmap glyph image.  This is */
  /*    a sub-class of @FT_Glyph, and a pointer to @FT_BitmapGlyphRec.     */
  /*                                                                       */
  typedef struct FT_BitmapGlyphRec_*  FT_BitmapGlyph;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_BitmapGlyphRec                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used for bitmap glyph images.  This really is a        */
  /*    `sub-class' of @FT_GlyphRec.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    root   :: The root @FT_Glyph fields.                               */
  /*                                                                       */
  /*    left   :: The left-side bearing, i.e., the horizontal distance     */
  /*              from the current pen position to the left border of the  */
  /*              glyph bitmap.                                            */
  /*                                                                       */
  /*    top    :: The top-side bearing, i.e., the vertical distance from   */
  /*              the current pen position to the top border of the glyph  */
  /*              bitmap.  This distance is positive for upwards~y!        */
  /*                                                                       */
  /*    bitmap :: A descriptor for the bitmap.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You can typecast an @FT_Glyph to @FT_BitmapGlyph if you have       */
  /*    `glyph->format == FT_GLYPH_FORMAT_BITMAP'.  This lets you access   */
  /*    the bitmap's contents easily.                                      */
  /*                                                                       */
  /*    The corresponding pixel buffer is always owned by @FT_BitmapGlyph  */
  /*    and is thus created and destroyed with it.                         */
  /*                                                                       */
  typedef struct  FT_BitmapGlyphRec_
  {
	FT_GlyphRec  root;
	FT_Int       left;
	FT_Int       top;
	FT_Bitmap    bitmap;

  } FT_BitmapGlyphRec;

  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_OutlineGlyph                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an object used to model an outline glyph image.  This  */
  /*    is a sub-class of @FT_Glyph, and a pointer to @FT_OutlineGlyphRec. */
  /*                                                                       */
  typedef struct FT_OutlineGlyphRec_*  FT_OutlineGlyph;

  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_OutlineGlyphRec                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used for outline (vectorial) glyph images.  This       */
  /*    really is a `sub-class' of @FT_GlyphRec.                           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    root    :: The root @FT_Glyph fields.                              */
  /*                                                                       */
  /*    outline :: A descriptor for the outline.                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You can typecast an @FT_Glyph to @FT_OutlineGlyph if you have      */
  /*    `glyph->format == FT_GLYPH_FORMAT_OUTLINE'.  This lets you access  */
  /*    the outline's content easily.                                      */
  /*                                                                       */
  /*    As the outline is extracted from a glyph slot, its coordinates are */
  /*    expressed normally in 26.6 pixels, unless the flag                 */
  /*    @FT_LOAD_NO_SCALE was used in @FT_Load_Glyph() or @FT_Load_Char(). */
  /*                                                                       */
  /*    The outline's tables are always owned by the object and are        */
  /*    destroyed with it.                                                 */
  /*                                                                       */
  typedef struct  FT_OutlineGlyphRec_
  {
	FT_GlyphRec  root;
	FT_Outline   outline;

  } FT_OutlineGlyphRec;

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Glyph                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to extract a glyph image from a slot.  Note that   */
  /*    the created @FT_Glyph object must be released with @FT_Done_Glyph. */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot   :: A handle to the source glyph slot.                       */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aglyph :: A handle to the glyph object.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Get_Glyph( FT_GlyphSlot  slot,
				FT_Glyph     *aglyph );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Glyph_Copy                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A function used to copy a glyph image.  Note that the created      */
  /*    @FT_Glyph object must be released with @FT_Done_Glyph.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    source :: A handle to the source glyph object.                     */
  /*                                                                       */
  /* <Output>                                                              */
  /*    target :: A handle to the target glyph object.  0~in case of       */
  /*              error.                                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Glyph_Copy( FT_Glyph   source,
				 FT_Glyph  *target );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Glyph_Transform                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Transform a glyph image if its format is scalable.                 */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    glyph  :: A handle to the target glyph object.                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    matrix :: A pointer to a 2x2 matrix to apply.                      */
  /*                                                                       */
  /*    delta  :: A pointer to a 2d vector to apply.  Coordinates are      */
  /*              expressed in 1/64th of a pixel.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code (if not 0, the glyph format is not scalable).  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The 2x2 transformation matrix is also applied to the glyph's       */
  /*    advance vector.                                                    */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Glyph_Transform( FT_Glyph    glyph,
					  FT_Matrix*  matrix,
					  FT_Vector*  delta );

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    FT_Glyph_BBox_Mode                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The mode how the values of @FT_Glyph_Get_CBox are returned.        */
  /*                                                                       */
  /* <Values>                                                              */
  /*    FT_GLYPH_BBOX_UNSCALED ::                                          */
  /*      Return unscaled font units.                                      */
  /*                                                                       */
  /*    FT_GLYPH_BBOX_SUBPIXELS ::                                         */
  /*      Return unfitted 26.6 coordinates.                                */
  /*                                                                       */
  /*    FT_GLYPH_BBOX_GRIDFIT ::                                           */
  /*      Return grid-fitted 26.6 coordinates.                             */
  /*                                                                       */
  /*    FT_GLYPH_BBOX_TRUNCATE ::                                          */
  /*      Return coordinates in integer pixels.                            */
  /*                                                                       */
  /*    FT_GLYPH_BBOX_PIXELS ::                                            */
  /*      Return grid-fitted pixel coordinates.                            */
  /*                                                                       */
  typedef enum  FT_Glyph_BBox_Mode_
  {
	FT_GLYPH_BBOX_UNSCALED  = 0,
	FT_GLYPH_BBOX_SUBPIXELS = 0,
	FT_GLYPH_BBOX_GRIDFIT   = 1,
	FT_GLYPH_BBOX_TRUNCATE  = 2,
	FT_GLYPH_BBOX_PIXELS    = 3

  } FT_Glyph_BBox_Mode;

  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    ft_glyph_bbox_xxx                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    These constants are deprecated.  Use the corresponding             */
  /*    @FT_Glyph_BBox_Mode values instead.                                */
  /*                                                                       */
  /* <Values>                                                              */
  /*   ft_glyph_bbox_unscaled  :: See @FT_GLYPH_BBOX_UNSCALED.             */
  /*   ft_glyph_bbox_subpixels :: See @FT_GLYPH_BBOX_SUBPIXELS.            */
  /*   ft_glyph_bbox_gridfit   :: See @FT_GLYPH_BBOX_GRIDFIT.              */
  /*   ft_glyph_bbox_truncate  :: See @FT_GLYPH_BBOX_TRUNCATE.             */
  /*   ft_glyph_bbox_pixels    :: See @FT_GLYPH_BBOX_PIXELS.               */
  /*                                                                       */
#define ft_glyph_bbox_unscaled   FT_GLYPH_BBOX_UNSCALED
#define ft_glyph_bbox_subpixels  FT_GLYPH_BBOX_SUBPIXELS
#define ft_glyph_bbox_gridfit    FT_GLYPH_BBOX_GRIDFIT
#define ft_glyph_bbox_truncate   FT_GLYPH_BBOX_TRUNCATE
#define ft_glyph_bbox_pixels     FT_GLYPH_BBOX_PIXELS

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Glyph_Get_CBox                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return a glyph's `control box'.  The control box encloses all the  */
  /*    outline's points, including Bézier control points.  Though it      */
  /*    coincides with the exact bounding box for most glyphs, it can be   */
  /*    slightly larger in some situations (like when rotating an outline  */
  /*    which contains Bézier outside arcs).                               */
  /*                                                                       */
  /*    Computing the control box is very fast, while getting the bounding */
  /*    box can take much more time as it needs to walk over all segments  */
  /*    and arcs in the outline.  To get the latter, you can use the       */
  /*    `ftbbox' component which is dedicated to this single task.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    glyph :: A handle to the source glyph object.                      */
  /*                                                                       */
  /*    mode  :: The mode which indicates how to interpret the returned    */
  /*             bounding box values.                                      */
  /*                                                                       */
  /* <Output>                                                              */
  /*    acbox :: The glyph coordinate bounding box.  Coordinates are       */
  /*             expressed in 1/64th of pixels if it is grid-fitted.       */
  /*                                                                       */
  /* <Note>                                                                */
  /*    Coordinates are relative to the glyph origin, using the y~upwards  */
  /*    convention.                                                        */
  /*                                                                       */
  /*    If the glyph has been loaded with @FT_LOAD_NO_SCALE, `bbox_mode'   */
  /*    must be set to @FT_GLYPH_BBOX_UNSCALED to get unscaled font        */
  /*    units in 26.6 pixel format.  The value @FT_GLYPH_BBOX_SUBPIXELS    */
  /*    is another name for this constant.                                 */
  /*                                                                       */
  /*    If the font is tricky and the glyph has been loaded with           */
  /*    @FT_LOAD_NO_SCALE, the resulting CBox is meaningless.  To get      */
  /*    reasonable values for the CBox it is necessary to load the glyph   */
  /*    at a large ppem value (so that the hinting instructions can        */
  /*    properly shift and scale the subglyphs), then extracting the CBox  */
  /*    which can be eventually converted back to font units.              */
  /*                                                                       */
  /*    Note that the maximum coordinates are exclusive, which means that  */
  /*    one can compute the width and height of the glyph image (be it in  */
  /*    integer or 26.6 pixels) as:                                        */
  /*                                                                       */
  /*    {                                                                  */
  /*      width  = bbox.xMax - bbox.xMin;                                  */
  /*      height = bbox.yMax - bbox.yMin;                                  */
  /*    }                                                                  */
  /*                                                                       */
  /*    Note also that for 26.6 coordinates, if `bbox_mode' is set to      */
  /*    @FT_GLYPH_BBOX_GRIDFIT, the coordinates will also be grid-fitted,  */
  /*    which corresponds to:                                              */
  /*                                                                       */
  /*    {                                                                  */
  /*      bbox.xMin = FLOOR(bbox.xMin);                                    */
  /*      bbox.yMin = FLOOR(bbox.yMin);                                    */
  /*      bbox.xMax = CEILING(bbox.xMax);                                  */
  /*      bbox.yMax = CEILING(bbox.yMax);                                  */
  /*    }                                                                  */
  /*                                                                       */
  /*    To get the bbox in pixel coordinates, set `bbox_mode' to           */
  /*    @FT_GLYPH_BBOX_TRUNCATE.                                           */
  /*                                                                       */
  /*    To get the bbox in grid-fitted pixel coordinates, set `bbox_mode'  */
  /*    to @FT_GLYPH_BBOX_PIXELS.                                          */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Glyph_Get_CBox( FT_Glyph  glyph,
					 FT_UInt   bbox_mode,
					 FT_BBox  *acbox );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Glyph_To_Bitmap                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Convert a given glyph object to a bitmap glyph object.             */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    the_glyph   :: A pointer to a handle to the target glyph.          */
  /*                                                                       */
  /* <Input>                                                               */
  /*    render_mode :: An enumeration that describes how the data is       */
  /*                   rendered.                                           */
  /*                                                                       */
  /*    origin      :: A pointer to a vector used to translate the glyph   */
  /*                   image before rendering.  Can be~0 (if no            */
  /*                   translation).  The origin is expressed in           */
  /*                   26.6 pixels.                                        */
  /*                                                                       */
  /*    destroy     :: A boolean that indicates that the original glyph    */
  /*                   image should be destroyed by this function.  It is  */
  /*                   never destroyed in case of error.                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function does nothing if the glyph format isn't scalable.     */
  /*                                                                       */
  /*    The glyph image is translated with the `origin' vector before      */
  /*    rendering.                                                         */
  /*                                                                       */
  /*    The first parameter is a pointer to an @FT_Glyph handle, that will */
  /*    be _replaced_ by this function (with newly allocated data).        */
  /*    Typically, you would use (omitting error handling):                */
  /*                                                                       */
  /*                                                                       */
  /*      {                                                                */
  /*        FT_Glyph        glyph;                                         */
  /*        FT_BitmapGlyph  glyph_bitmap;                                  */
  /*                                                                       */
  /*                                                                       */
  /*        // load glyph                                                  */
  /*        error = FT_Load_Char( face, glyph_index, FT_LOAD_DEFAUT );     */
  /*                                                                       */
  /*        // extract glyph image                                         */
  /*        error = FT_Get_Glyph( face->glyph, &glyph );                   */
  /*                                                                       */
  /*        // convert to a bitmap (default render mode + destroying old)  */
  /*        if ( glyph->format != FT_GLYPH_FORMAT_BITMAP )                 */
  /*        {                                                              */
  /*          error = FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL,   */
  /*                                      0, 1 );                          */
  /*          if ( error ) // `glyph' unchanged                            */
  /*            ...                                                        */
  /*        }                                                              */
  /*                                                                       */
  /*        // access bitmap content by typecasting                        */
  /*        glyph_bitmap = (FT_BitmapGlyph)glyph;                          */
  /*                                                                       */
  /*        // do funny stuff with it, like blitting/drawing               */
  /*        ...                                                            */
  /*                                                                       */
  /*        // discard glyph image (bitmap or not)                         */
  /*        FT_Done_Glyph( glyph );                                        */
  /*      }                                                                */
  /*                                                                       */
  /*                                                                       */
  /*    Here another example, again without error handling:                */
  /*                                                                       */
  /*                                                                       */
  /*      {                                                                */
  /*        FT_Glyph  glyphs[MAX_GLYPHS]                                   */
  /*                                                                       */
  /*                                                                       */
  /*        ...                                                            */
  /*                                                                       */
  /*        for ( idx = 0; i < MAX_GLYPHS; i++ )                           */
  /*          error = FT_Load_Glyph( face, idx, FT_LOAD_DEFAULT ) ||       */
  /*                  FT_Get_Glyph ( face->glyph, &glyph[idx] );           */
  /*                                                                       */
  /*        ...                                                            */
  /*                                                                       */
  /*        for ( idx = 0; i < MAX_GLYPHS; i++ )                           */
  /*        {                                                              */
  /*          FT_Glyph  bitmap = glyphs[idx];                              */
  /*                                                                       */
  /*                                                                       */
  /*          ...                                                          */
  /*                                                                       */
  /*          // after this call, `bitmap' no longer points into           */
  /*          // the `glyphs' array (and the old value isn't destroyed)    */
  /*          FT_Glyph_To_Bitmap( &bitmap, FT_RENDER_MODE_MONO, 0, 0 );    */
  /*                                                                       */
  /*          ...                                                          */
  /*                                                                       */
  /*          FT_Done_Glyph( bitmap );                                     */
  /*        }                                                              */
  /*                                                                       */
  /*        ...                                                            */
  /*                                                                       */
  /*        for ( idx = 0; i < MAX_GLYPHS; i++ )                           */
  /*          FT_Done_Glyph( glyphs[idx] );                                */
  /*      }                                                                */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Glyph_To_Bitmap( FT_Glyph*       the_glyph,
					  FT_Render_Mode  render_mode,
					  FT_Vector*      origin,
					  FT_Bool         destroy );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Done_Glyph                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroy a given glyph.                                             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    glyph :: A handle to the target glyph object.                      */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Done_Glyph( FT_Glyph  glyph );

  /* */

  /* other helpful functions */

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    computations                                                       */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Matrix_Multiply                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Perform the matrix operation `b = a*b'.                            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    a :: A pointer to matrix `a'.                                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    b :: A pointer to matrix `b'.                                      */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The result is undefined if either `a' or `b' is zero.              */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Matrix_Multiply( const FT_Matrix*  a,
					  FT_Matrix*        b );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Matrix_Invert                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Invert a 2x2 matrix.  Return an error if it can't be inverted.     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    matrix :: A pointer to the target matrix.  Remains untouched in    */
  /*              case of error.                                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Matrix_Invert( FT_Matrix*  matrix );

  /* */

FT_END_HEADER

#endif /* __FTGLYPH_H__ */

/* END */

/* Local Variables: */
/* coding: utf-8    */
/* End:             */

/*** End of inlined file: ftglyph.h ***/


/*** Start of inlined file: ftoutln.h ***/
/*                                                                         */
/*  ftoutln.h                                                              */
/*                                                                         */
/*    Support for the FT_Outline type used to store glyph shapes of        */
/*    most scalable font formats (specification).                          */
/*                                                                         */
/*  Copyright 1996-2003, 2005-2012 by                                      */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/

#ifndef __FTOUTLN_H__
#define __FTOUTLN_H__

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif

FT_BEGIN_HEADER

  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    outline_processing                                                 */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Outline Processing                                                 */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Functions to create, transform, and render vectorial glyph images. */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains routines used to create and destroy scalable */
  /*    glyph images known as `outlines'.  These can also be measured,     */
  /*    transformed, and converted into bitmaps and pixmaps.               */
  /*                                                                       */
  /* <Order>                                                               */
  /*    FT_Outline                                                         */
  /*    FT_OUTLINE_FLAGS                                                   */
  /*    FT_Outline_New                                                     */
  /*    FT_Outline_Done                                                    */
  /*    FT_Outline_Copy                                                    */
  /*    FT_Outline_Translate                                               */
  /*    FT_Outline_Transform                                               */
  /*    FT_Outline_Embolden                                                */
  /*    FT_Outline_EmboldenXY                                              */
  /*    FT_Outline_Reverse                                                 */
  /*    FT_Outline_Check                                                   */
  /*                                                                       */
  /*    FT_Outline_Get_CBox                                                */
  /*    FT_Outline_Get_BBox                                                */
  /*                                                                       */
  /*    FT_Outline_Get_Bitmap                                              */
  /*    FT_Outline_Render                                                  */
  /*                                                                       */
  /*    FT_Outline_Decompose                                               */
  /*    FT_Outline_Funcs                                                   */
  /*    FT_Outline_MoveTo_Func                                             */
  /*    FT_Outline_LineTo_Func                                             */
  /*    FT_Outline_ConicTo_Func                                            */
  /*    FT_Outline_CubicTo_Func                                            */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Decompose                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Walk over an outline's structure to decompose it into individual   */
  /*    segments and Bézier arcs.  This function also emits `move to'      */
  /*    operations to indicate the start of new contours in the outline.   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline        :: A pointer to the source target.                  */
  /*                                                                       */
  /*    func_interface :: A table of `emitters', i.e., function pointers   */
  /*                      called during decomposition to indicate path     */
  /*                      operations.                                      */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    user           :: A typeless pointer which is passed to each       */
  /*                      emitter during the decomposition.  It can be     */
  /*                      used to store the state during the               */
  /*                      decomposition.                                   */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_Decompose( FT_Outline*              outline,
						const FT_Outline_Funcs*  func_interface,
						void*                    user );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_New                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Create a new outline of a given size.                              */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library     :: A handle to the library object from where the       */
  /*                   outline is allocated.  Note however that the new    */
  /*                   outline will *not* necessarily be *freed*, when     */
  /*                   destroying the library, by @FT_Done_FreeType.       */
  /*                                                                       */
  /*    numPoints   :: The maximum number of points within the outline.    */
  /*                                                                       */
  /*    numContours :: The maximum number of contours within the outline.  */
  /*                                                                       */
  /* <Output>                                                              */
  /*    anoutline   :: A handle to the new outline.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The reason why this function takes a `library' parameter is simply */
  /*    to use the library's memory allocator.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_New( FT_Library   library,
				  FT_UInt      numPoints,
				  FT_Int       numContours,
				  FT_Outline  *anoutline );

  FT_EXPORT( FT_Error )
  FT_Outline_New_Internal( FT_Memory    memory,
						   FT_UInt      numPoints,
						   FT_Int       numContours,
						   FT_Outline  *anoutline );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Done                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroy an outline created with @FT_Outline_New.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle of the library object used to allocate the     */
  /*               outline.                                                */
  /*                                                                       */
  /*    outline :: A pointer to the outline object to be discarded.        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    If the outline's `owner' field is not set, only the outline        */
  /*    descriptor will be released.                                       */
  /*                                                                       */
  /*    The reason why this function takes an `library' parameter is       */
  /*    simply to use ft_mem_free().                                       */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_Done( FT_Library   library,
				   FT_Outline*  outline );

  FT_EXPORT( FT_Error )
  FT_Outline_Done_Internal( FT_Memory    memory,
							FT_Outline*  outline );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Check                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Check the contents of an outline descriptor.                       */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A handle to a source outline.                           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_Check( FT_Outline*  outline );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Get_CBox                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Return an outline's `control box'.  The control box encloses all   */
  /*    the outline's points, including Bézier control points.  Though it  */
  /*    coincides with the exact bounding box for most glyphs, it can be   */
  /*    slightly larger in some situations (like when rotating an outline  */
  /*    which contains Bézier outside arcs).                               */
  /*                                                                       */
  /*    Computing the control box is very fast, while getting the bounding */
  /*    box can take much more time as it needs to walk over all segments  */
  /*    and arcs in the outline.  To get the latter, you can use the       */
  /*    `ftbbox' component which is dedicated to this single task.         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    acbox   :: The outline's control box.                              */
  /*                                                                       */
  /* <Note>                                                                */
  /*    See @FT_Glyph_Get_CBox for a discussion of tricky fonts.           */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Outline_Get_CBox( const FT_Outline*  outline,
					   FT_BBox           *acbox );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Translate                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Apply a simple translation to the points of an outline.            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    outline :: A pointer to the target outline descriptor.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    xOffset :: The horizontal offset.                                  */
  /*                                                                       */
  /*    yOffset :: The vertical offset.                                    */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Outline_Translate( const FT_Outline*  outline,
						FT_Pos             xOffset,
						FT_Pos             yOffset );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Copy                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Copy an outline into another one.  Both objects must have the      */
  /*    same sizes (number of points & number of contours) when this       */
  /*    function is called.                                                */
  /*                                                                       */
  /* <Input>                                                               */
  /*    source :: A handle to the source outline.                          */
  /*                                                                       */
  /* <Output>                                                              */
  /*    target :: A handle to the target outline.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_Copy( const FT_Outline*  source,
				   FT_Outline        *target );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Transform                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Apply a simple 2x2 matrix to all of an outline's points.  Useful   */
  /*    for applying rotations, slanting, flipping, etc.                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    outline :: A pointer to the target outline descriptor.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    matrix  :: A pointer to the transformation matrix.                 */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You can use @FT_Outline_Translate if you need to translate the     */
  /*    outline's points.                                                  */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Outline_Transform( const FT_Outline*  outline,
						const FT_Matrix*   matrix );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Embolden                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Embolden an outline.  The new outline will be at most 4~times      */
  /*    `strength' pixels wider and higher.  You may think of the left and */
  /*    bottom borders as unchanged.                                       */
  /*                                                                       */
  /*    Negative `strength' values to reduce the outline thickness are     */
  /*    possible also.                                                     */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    outline  :: A handle to the target outline.                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    strength :: How strong the glyph is emboldened.  Expressed in      */
  /*                26.6 pixel format.                                     */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The used algorithm to increase or decrease the thickness of the    */
  /*    glyph doesn't change the number of points; this means that certain */
  /*    situations like acute angles or intersections are sometimes        */
  /*    handled incorrectly.                                               */
  /*                                                                       */
  /*    If you need `better' metrics values you should call                */
  /*    @FT_Outline_Get_CBox or @FT_Outline_Get_BBox.                      */
  /*                                                                       */
  /*    Example call:                                                      */
  /*                                                                       */
  /*    {                                                                  */
  /*      FT_Load_Glyph( face, index, FT_LOAD_DEFAULT );                   */
  /*      if ( face->slot->format == FT_GLYPH_FORMAT_OUTLINE )             */
  /*        FT_Outline_Embolden( &face->slot->outline, strength );         */
  /*    }                                                                  */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_Embolden( FT_Outline*  outline,
					   FT_Pos       strength );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_EmboldenXY                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Embolden an outline.  The new outline will be `xstrength' pixels   */
  /*    wider and `ystrength' pixels higher.  Otherwise, it is similar to  */
  /*    @FT_Outline_Embolden, which uses the same strength in both         */
  /*    directions.                                                        */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_EmboldenXY( FT_Outline*  outline,
						 FT_Pos       xstrength,
						 FT_Pos       ystrength );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Reverse                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Reverse the drawing direction of an outline.  This is used to      */
  /*    ensure consistent fill conventions for mirrored glyphs.            */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    outline :: A pointer to the target outline descriptor.             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function toggles the bit flag @FT_OUTLINE_REVERSE_FILL in     */
  /*    the outline's `flags' field.                                       */
  /*                                                                       */
  /*    It shouldn't be used by a normal client application, unless it     */
  /*    knows what it is doing.                                            */
  /*                                                                       */
  FT_EXPORT( void )
  FT_Outline_Reverse( FT_Outline*  outline );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Get_Bitmap                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Render an outline within a bitmap.  The outline's image is simply  */
  /*    OR-ed to the target bitmap.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a FreeType library object.                  */
  /*                                                                       */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    abitmap :: A pointer to the target bitmap descriptor.              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function does NOT CREATE the bitmap, it only renders an       */
  /*    outline image within the one you pass to it!  Consequently, the    */
  /*    various fields in `abitmap' should be set accordingly.             */
  /*                                                                       */
  /*    It will use the raster corresponding to the default glyph format.  */
  /*                                                                       */
  /*    The value of the `num_grays' field in `abitmap' is ignored.  If    */
  /*    you select the gray-level rasterizer, and you want less than 256   */
  /*    gray levels, you have to use @FT_Outline_Render directly.          */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_Get_Bitmap( FT_Library        library,
						 FT_Outline*       outline,
						 const FT_Bitmap  *abitmap );

  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Outline_Render                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Render an outline within a bitmap using the current scan-convert.  */
  /*    This function uses an @FT_Raster_Params structure as an argument,  */
  /*    allowing advanced features like direct composition, translucency,  */
  /*    etc.                                                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a FreeType library object.                  */
  /*                                                                       */
  /*    outline :: A pointer to the source outline descriptor.             */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    params  :: A pointer to an @FT_Raster_Params structure used to     */
  /*               describe the rendering operation.                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    You should know what you are doing and how @FT_Raster_Params works */
  /*    to use this function.                                              */
  /*                                                                       */
  /*    The field `params.source' will be set to `outline' before the scan */
  /*    converter is called, which means that the value you give to it is  */
  /*    actually ignored.                                                  */
  /*                                                                       */
  /*    The gray-level rasterizer always uses 256 gray levels.  If you     */
  /*    want less gray levels, you have to provide your own span callback. */
  /*    See the @FT_RASTER_FLAG_DIRECT value of the `flags' field in the   */
  /*    @FT_Raster_Params structure for more details.                      */
  /*                                                                       */
  FT_EXPORT( FT_Error )
  FT_Outline_Render( FT_Library         library,
					 FT_Outline*        outline,
					 FT_Raster_Params*  params );

 /**************************************************************************
  *
  * @enum:
  *   FT_Orientation
  *
  * @description:
  *   A list of values used to describe an outline's contour orientation.
  *
  *   The TrueType and PostScript specifications use different conventions
  *   to determine whether outline contours should be filled or unfilled.
  *
  * @values:
  *   FT_ORIENTATION_TRUETYPE ::
  *     According to the TrueType specification, clockwise contours must
  *     be filled, and counter-clockwise ones must be unfilled.
  *
  *   FT_ORIENTATION_POSTSCRIPT ::
  *     According to the PostScript specification, counter-clockwise contours
  *     must be filled, and clockwise ones must be unfilled.
  *
  *   FT_ORIENTATION_FILL_RIGHT ::
  *     This is identical to @FT_ORIENTATION_TRUETYPE, but is used to
  *     remember that in TrueType, everything that is to the right of
  *     the drawing direction of a contour must be filled.
  *
  *   FT_ORIENTATION_FILL_LEFT ::
  *     This is identical to @FT_ORIENTATION_POSTSCRIPT, but is used to
  *     remember that in PostScript, everything that is to the left of
  *     the drawing direction of a contour must be filled.
  *
  *   FT_ORIENTATION_NONE ::
  *     The orientation cannot be determined.  That is, different parts of
  *     the glyph have different orientation.
  *
  */
  typedef enum  FT_Orientation_
  {
	FT_ORIENTATION_TRUETYPE   = 0,
	FT_ORIENTATION_POSTSCRIPT = 1,
	FT_ORIENTATION_FILL_RIGHT = FT_ORIENTATION_TRUETYPE,
	FT_ORIENTATION_FILL_LEFT  = FT_ORIENTATION_POSTSCRIPT,
	FT_ORIENTATION_NONE

  } FT_Orientation;

 /**************************************************************************
  *
  * @function:
  *   FT_Outline_Get_Orientation
  *
  * @description:
  *   This function analyzes a glyph outline and tries to compute its
  *   fill orientation (see @FT_Orientation).  This is done by computing
  *   the direction of each global horizontal and/or vertical extrema
  *   within the outline.
  *
  *   Note that this will return @FT_ORIENTATION_TRUETYPE for empty
  *   outlines.
  *
  * @input:
  *   outline ::
  *     A handle to the source outline.
  *
  * @return:
  *   The orientation.
  *
  */
  FT_EXPORT( FT_Orientation )
  FT_Outline_Get_Orientation( FT_Outline*  outline );

  /* */

FT_END_HEADER

#endif /* __FTOUTLN_H__ */

/* END */

/* Local Variables: */
/* coding: utf-8    */
/* End:             */

/*** End of inlined file: ftoutln.h ***/

#ifdef _MSC_VER
#pragma pop_macro("_CRT_SECURE_NO_WARNINGS")
#endif

