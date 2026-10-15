#ifdef HB_FEATURES_H
#include HB_FEATURES_H
#endif

#ifdef HB_HAS_CORETEXT
#define HAVE_CORETEXT 1
#endif
#ifdef HB_HAS_DIRECTWRITE
#define HAVE_DIRECTWRITE 1
#endif
#ifdef HB_HAS_FREETYPE
#define HAVE_FREETYPE 1
#endif
#ifdef HB_HAS_GDI
#define HAVE_GDI 1
#endif
#ifdef HB_HAS_GLIB
#define HAVE_GLIB 1
#endif
#ifdef HB_HAS_GRAPHITE
#define HAVE_GRAPHITE 1
#endif
#ifdef HB_HAS_UNISCRIBE
#define HAVE_UNISCRIBE 1
#define HAVE_GDI 1
#endif
#ifdef HB_HAS_WASM
#define HAVE_WASM 1
#endif


/* Core library. */
#include "OT/Var/VARC/VARC.cc"
#include "hb-aat-layout.cc"
#include "hb-aat-map.cc"
#include "hb-blob.cc"
#include "hb-buffer-serialize.cc"
#include "hb-buffer-verify.cc"
#include "hb-buffer.cc"
#include "hb-common.cc"
#include "hb-draw.cc"
#include "hb-face-builder.cc"
#include "hb-face.cc"
#include "hb-fallback-shape.cc"
#include "hb-font.cc"
#include "hb-map.cc"
#include "hb-number.cc"
#include "hb-ot-cff1-table.cc"
#include "hb-ot-cff2-table.cc"
#include "hb-ot-color.cc"
#include "hb-ot-face.cc"
#include "hb-ot-fetch.cc"
#include "hb-ot-font.cc"
#include "hb-ot-layout.cc"
#include "hb-ot-map.cc"
#include "hb-ot-math.cc"
#include "hb-ot-meta.cc"
#include "hb-ot-metrics.cc"
#include "hb-ot-name.cc"
#include "hb-ot-shape-fallback.cc"
#include "hb-ot-shape-normalize.cc"
#include "hb-ot-shape.cc"
#include "hb-ot-shaper-arabic.cc"
#include "hb-ot-shaper-default.cc"
#include "hb-ot-shaper-hangul.cc"
#include "hb-ot-shaper-hebrew.cc"
#include "hb-ot-shaper-indic-table.cc"
#include "hb-ot-shaper-indic.cc"
#include "hb-ot-shaper-khmer.cc"
#include "hb-ot-shaper-myanmar.cc"
#include "hb-ot-shaper-syllabic.cc"
#include "hb-ot-shaper-thai.cc"
#include "hb-ot-shaper-use.cc"
#include "hb-ot-shaper-vowel-constraints.cc"
#include "hb-ot-tag.cc"
#include "hb-ot-var.cc"
#include "hb-outline.cc"
#include "hb-paint-bounded.cc"
#include "hb-paint-extents.cc"
#include "hb-paint.cc"
#include "hb-set.cc"
#include "hb-shape-plan.cc"
#include "hb-shape.cc"
#include "hb-shaper.cc"
#include "hb-static.cc"
#include "hb-style.cc"
#include "hb-ucd.cc"
#include "hb-unicode.cc"

#ifdef HB_HAS_CAIRO
#include "hb-cairo-utils.cc"
#include "hb-cairo.cc"
#include "hb-static.cc"
#endif

#ifdef HB_HAS_CORETEXT
#include "hb-coretext-font.cc"
#include "hb-coretext-shape.cc"
#include "hb-coretext.cc"
#endif

#ifdef HB_HAS_DIRECTWRITE
#include "hb-directwrite-font.cc"
#include "hb-directwrite-shape.cc"
#include "hb-directwrite.cc"
#endif

#ifdef HB_HAS_FREETYPE
#include "hb-ft.cc"
#endif

#ifdef HB_HAS_GDI
#ifndef HAVE_GDI
#include "hb-gdi.cc"
#endif
#endif

#ifdef HB_HAS_GLIB
#include "hb-glib.cc"
#endif

#ifdef HB_HAS_GRAPHITE
#include "hb-graphite2.cc"
#endif

#ifdef HB_HAS_GPU
#include "hb-gpu-draw.cc"
#include "hb-gpu-paint.cc"
#include "hb-gpu.cc"
#include "hb-static.cc"
#endif

#ifdef HB_HAS_ICU
#include "hb-icu.cc"
#endif

#ifdef HB_HAS_RASTER
#include "hb-raster-draw.cc"
#include "hb-raster-image.cc"
#include "hb-raster-paint.cc"
#include "hb-static.cc"
#endif

#ifdef HB_HAS_SUBSET
#include "OT/Var/VARC/VARC.cc"
#include "graph/gsubgpos-context.cc"
#include "hb-depend.cc"
#include "hb-number.cc"
#include "hb-ot-cff1-table.cc"
#include "hb-ot-cff2-table.cc"
#include "hb-static.cc"
#include "hb-subset-cff-common.cc"
#include "hb-subset-cff1.cc"
#include "hb-subset-cff2.cc"
#include "hb-subset-input.cc"
#include "hb-subset-instancer-iup.cc"
#include "hb-subset-instancer-solver.cc"
#include "hb-subset-plan-layout.cc"
#include "hb-subset-plan-var.cc"
#include "hb-subset-plan.cc"
#include "hb-subset-serialize.cc"
#include "hb-subset-table-cff.cc"
#include "hb-subset-table-color.cc"
#include "hb-subset-table-layout.cc"
#include "hb-subset-table-other.cc"
#include "hb-subset-table-var.cc"
#include "hb-subset.cc"
#endif

#ifdef HB_HAS_UNISCRIBE
#include "hb-uniscribe.cc"
#endif

#ifdef HB_HAS_VECTOR
#include "hb-static.cc"
#include "hb-vector-draw.cc"
#include "hb-vector-paint-pdf.cc"
#include "hb-vector-paint-svg.cc"
#include "hb-vector-paint.cc"
#include "hb-vector-path.cc"
#include "hb-vector.cc"
#include "hb-zlib.cc"
#endif

#ifdef HB_HAS_WASM
#include "hb-wasm-api.cc"
#include "hb-wasm-shape.cc"
#endif
