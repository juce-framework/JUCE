#ifndef JUCE_PLUTOVG_PRIVATE_H
#define JUCE_PLUTOVG_PRIVATE_H

#include "../include/plutovg.h"

#if defined(_WIN32)

#include <windows.h>

typedef LONG juce_plutovg_ref_count_t;

#define juce_plutovg_init_reference(ob) ((ob)->ref_count = 1)
#define juce_plutovg_increment_reference(ob) (void)(ob && InterlockedIncrement(&(ob)->ref_count))
#define juce_plutovg_destroy_reference(ob) (ob && InterlockedDecrement(&(ob)->ref_count) == 0)
#define juce_plutovg_get_reference_count(ob) ((ob) ? InterlockedCompareExchange((LONG*)&(ob)->ref_count, 0, 0) : 0)

#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)

#include <stdatomic.h>

typedef atomic_int juce_plutovg_ref_count_t;

#define juce_plutovg_init_reference(ob) atomic_init(&(ob)->ref_count, 1)
#define juce_plutovg_increment_reference(ob) (void)(ob && atomic_fetch_add(&(ob)->ref_count, 1))
#define juce_plutovg_destroy_reference(ob) (ob && atomic_fetch_sub(&(ob)->ref_count, 1) == 1)
#define juce_plutovg_get_reference_count(ob) ((ob) ? atomic_load(&(ob)->ref_count) : 0)

#else

typedef int juce_plutovg_ref_count_t;

#define juce_plutovg_init_reference(ob) ((ob)->ref_count = 1)
#define juce_plutovg_increment_reference(ob) (void)(ob && ++(ob)->ref_count)
#define juce_plutovg_destroy_reference(ob) (ob && --(ob)->ref_count == 0)
#define juce_plutovg_get_reference_count(ob) ((ob) ? (ob)->ref_count : 0)

#endif

struct juce_plutovg_surface {
    juce_plutovg_ref_count_t ref_count;
    int width;
    int height;
    int stride;
    unsigned char* data;
};

struct juce_plutovg_path {
    juce_plutovg_ref_count_t ref_count;
    int num_points;
    int num_contours;
    int num_curves;
    juce_plutovg_point_t start_point;
    struct {
        juce_plutovg_path_element_t* data;
        int size;
        int capacity;
    } elements;
};

typedef enum {
    JUCE_PLUTOVG_PAINT_TYPE_COLOR,
    JUCE_PLUTOVG_PAINT_TYPE_GRADIENT,
    JUCE_PLUTOVG_PAINT_TYPE_TEXTURE
} juce_plutovg_paint_type_t;

struct juce_plutovg_paint {
    juce_plutovg_ref_count_t ref_count;
    juce_plutovg_paint_type_t type;
};

typedef struct {
    juce_plutovg_paint_t base;
    juce_plutovg_color_t color;
} juce_plutovg_solid_paint_t;

typedef enum {
    JUCE_PLUTOVG_GRADIENT_TYPE_LINEAR,
    JUCE_PLUTOVG_GRADIENT_TYPE_RADIAL
} juce_plutovg_gradient_type_t;

typedef struct {
    juce_plutovg_paint_t base;
    juce_plutovg_gradient_type_t type;
    juce_plutovg_spread_method_t spread;
    juce_plutovg_matrix_t matrix;
    juce_plutovg_gradient_stop_t* stops;
    int nstops;
    float values[6];
} juce_plutovg_gradient_paint_t;

typedef struct {
    juce_plutovg_paint_t base;
    juce_plutovg_texture_type_t type;
    float opacity;
    juce_plutovg_matrix_t matrix;
    juce_plutovg_surface_t* surface;
} juce_plutovg_texture_paint_t;

typedef struct {
    int x;
    int len;
    int y;
    unsigned char coverage;
} juce_plutovg_span_t;

typedef struct {
    struct {
        juce_plutovg_span_t* data;
        int size;
        int capacity;
    } spans;

    int x;
    int y;
    int w;
    int h;
} juce_plutovg_span_buffer_t;

typedef struct {
    float offset;
    struct {
        float* data;
        int size;
        int capacity;
    } array;
} juce_plutovg_stroke_dash_t;

typedef struct {
    float width;
    juce_plutovg_line_cap_t cap;
    juce_plutovg_line_join_t join;
    float miter_limit;
} juce_plutovg_stroke_style_t;

typedef struct {
    juce_plutovg_stroke_style_t style;
    juce_plutovg_stroke_dash_t dash;
} juce_plutovg_stroke_data_t;

typedef struct juce_plutovg_state {
    juce_plutovg_paint_t* paint;
    juce_plutovg_font_face_t* font_face;
    juce_plutovg_color_t color;
    juce_plutovg_matrix_t matrix;
    juce_plutovg_stroke_data_t stroke;
    juce_plutovg_span_buffer_t clip_spans;
    juce_plutovg_fill_rule_t winding;
    juce_plutovg_operator_t op;
    float font_size;
    float opacity;
    bool clipping;
    struct juce_plutovg_state* next;
} juce_plutovg_state_t;

struct juce_plutovg_canvas {
    juce_plutovg_ref_count_t ref_count;
    juce_plutovg_surface_t* surface;
    juce_plutovg_path_t* path;
    juce_plutovg_state_t* state;
    juce_plutovg_state_t* freed_state;
    juce_plutovg_font_face_cache_t* face_cache;
    juce_plutovg_rect_t clip_rect;
    juce_plutovg_span_buffer_t clip_spans;
    juce_plutovg_span_buffer_t fill_spans;
};

void juce_plutovg_span_buffer_init(juce_plutovg_span_buffer_t* span_buffer);
void juce_plutovg_span_buffer_init_rect(juce_plutovg_span_buffer_t* span_buffer, int x, int y, int width, int height);
void juce_plutovg_span_buffer_reset(juce_plutovg_span_buffer_t* span_buffer);
void juce_plutovg_span_buffer_destroy(juce_plutovg_span_buffer_t* span_buffer);
void juce_plutovg_span_buffer_copy(juce_plutovg_span_buffer_t* span_buffer, const juce_plutovg_span_buffer_t* source);
bool juce_plutovg_span_buffer_contains(const juce_plutovg_span_buffer_t* span_buffer, float x, float y);
void juce_plutovg_span_buffer_extents(juce_plutovg_span_buffer_t* span_buffer, juce_plutovg_rect_t* extents);
void juce_plutovg_span_buffer_intersect(juce_plutovg_span_buffer_t* span_buffer, const juce_plutovg_span_buffer_t* a, const juce_plutovg_span_buffer_t* b);

void juce_plutovg_rasterize(juce_plutovg_span_buffer_t* span_buffer, const juce_plutovg_path_t* path, const juce_plutovg_matrix_t* matrix, const juce_plutovg_rect_t* clip_rect, const juce_plutovg_stroke_data_t* stroke_data, juce_plutovg_fill_rule_t winding);
void juce_plutovg_blend(juce_plutovg_canvas_t* canvas, const juce_plutovg_span_buffer_t* span_buffer);
void juce_plutovg_memfill32(unsigned int* dest, int length, unsigned int value);

#endif // JUCE_PLUTOVG_PRIVATE_H
