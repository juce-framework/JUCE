#include "plutovg-private.h"
#include "plutovg-utils.h"

int juce_plutovg_version(void)
{
    return JUCE_PLUTOVG_VERSION;
}

const char* juce_plutovg_version_string(void)
{
    return JUCE_PLUTOVG_VERSION_STRING;
}

#define JUCE_PLUTOVG_DEFAULT_STROKE_STYLE ((juce_plutovg_stroke_style_t){1.f, JUCE_PLUTOVG_LINE_CAP_BUTT, JUCE_PLUTOVG_LINE_JOIN_MITER, 10.f})

static juce_plutovg_state_t* juce_plutovg_state_create(void)
{
    juce_plutovg_state_t* state = malloc(sizeof(juce_plutovg_state_t));
    state->paint = NULL;
    state->font_face = NULL;
    state->color = JUCE_PLUTOVG_BLACK_COLOR;
    state->matrix = JUCE_PLUTOVG_IDENTITY_MATRIX;
    state->stroke.style = JUCE_PLUTOVG_DEFAULT_STROKE_STYLE;
    state->stroke.dash.offset = 0.f;
    juce_plutovg_array_init(state->stroke.dash.array);
    juce_plutovg_span_buffer_init(&state->clip_spans);
    state->winding = JUCE_PLUTOVG_FILL_RULE_NON_ZERO;
    state->op = JUCE_PLUTOVG_OPERATOR_SRC_OVER;
    state->font_size = 12.f;
    state->opacity = 1.f;
    state->clipping = false;
    state->next = NULL;
    return state;
}

static void juce_plutovg_state_reset(juce_plutovg_state_t* state)
{
    juce_plutovg_paint_destroy(state->paint);
    juce_plutovg_font_face_destroy(state->font_face);
    state->paint = NULL;
    state->font_face = NULL;
    state->color = JUCE_PLUTOVG_BLACK_COLOR;
    state->matrix = JUCE_PLUTOVG_IDENTITY_MATRIX;
    state->stroke.style = JUCE_PLUTOVG_DEFAULT_STROKE_STYLE;
    state->stroke.dash.offset = 0.f;
    juce_plutovg_array_clear(state->stroke.dash.array);
    juce_plutovg_span_buffer_reset(&state->clip_spans);
    state->winding = JUCE_PLUTOVG_FILL_RULE_NON_ZERO;
    state->op = JUCE_PLUTOVG_OPERATOR_SRC_OVER;
    state->font_size = 12.f;
    state->opacity = 1.f;
    state->clipping = false;
}

static void juce_plutovg_state_copy(juce_plutovg_state_t* state, const juce_plutovg_state_t* source)
{
    state->paint = juce_plutovg_paint_reference(source->paint);
    state->font_face = juce_plutovg_font_face_reference(source->font_face);
    state->color = source->color;
    state->matrix = source->matrix;
    state->stroke.style = source->stroke.style;
    state->stroke.dash.offset = source->stroke.dash.offset;
    juce_plutovg_array_clear(state->stroke.dash.array);
    juce_plutovg_array_append(state->stroke.dash.array, source->stroke.dash.array);
    juce_plutovg_span_buffer_copy(&state->clip_spans, &source->clip_spans);
    state->winding = source->winding;
    state->op = source->op;
    state->font_size = source->font_size;
    state->opacity = source->opacity;
    state->clipping = source->clipping;
}

static void juce_plutovg_state_destroy(juce_plutovg_state_t* state)
{
    juce_plutovg_paint_destroy(state->paint);
    juce_plutovg_font_face_destroy(state->font_face);
    juce_plutovg_array_destroy(state->stroke.dash.array);
    juce_plutovg_span_buffer_destroy(&state->clip_spans);
    free(state);
}

juce_plutovg_canvas_t* juce_plutovg_canvas_create(juce_plutovg_surface_t* surface)
{
    juce_plutovg_canvas_t* canvas = malloc(sizeof(juce_plutovg_canvas_t));
    juce_plutovg_init_reference(canvas);
    canvas->surface = juce_plutovg_surface_reference(surface);
    canvas->path = juce_plutovg_path_create();
    canvas->state = juce_plutovg_state_create();
    canvas->freed_state = NULL;
    canvas->face_cache = NULL;
    canvas->clip_rect = JUCE_PLUTOVG_MAKE_RECT(0, 0, surface->width, surface->height);
    juce_plutovg_span_buffer_init(&canvas->clip_spans);
    juce_plutovg_span_buffer_init(&canvas->fill_spans);
    return canvas;
}

juce_plutovg_canvas_t* juce_plutovg_canvas_reference(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_increment_reference(canvas);
    return canvas;
}

void juce_plutovg_canvas_destroy(juce_plutovg_canvas_t* canvas)
{
    if(juce_plutovg_destroy_reference(canvas)) {
        while(canvas->state) {
            juce_plutovg_state_t* state = canvas->state;
            canvas->state = state->next;
            juce_plutovg_state_destroy(state);
        }

        while(canvas->freed_state) {
            juce_plutovg_state_t* state = canvas->freed_state;
            canvas->freed_state = state->next;
            juce_plutovg_state_destroy(state);
        }

        juce_plutovg_font_face_cache_destroy(canvas->face_cache);
        juce_plutovg_span_buffer_destroy(&canvas->fill_spans);
        juce_plutovg_span_buffer_destroy(&canvas->clip_spans);
        juce_plutovg_surface_destroy(canvas->surface);
        juce_plutovg_path_destroy(canvas->path);
        free(canvas);
    }
}

int juce_plutovg_canvas_get_reference_count(const juce_plutovg_canvas_t* canvas)
{
    return juce_plutovg_get_reference_count(canvas);
}

juce_plutovg_surface_t* juce_plutovg_canvas_get_surface(const juce_plutovg_canvas_t* canvas)
{
    return canvas->surface;
}

void juce_plutovg_canvas_save(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_state_t* new_state = canvas->freed_state;
    if(new_state == NULL)
        new_state = juce_plutovg_state_create();
    else
        canvas->freed_state = new_state->next;
    juce_plutovg_state_copy(new_state, canvas->state);
    new_state->next = canvas->state;
    canvas->state = new_state;
}

void juce_plutovg_canvas_restore(juce_plutovg_canvas_t* canvas)
{
    if(canvas->state->next == NULL)
        return;
    juce_plutovg_state_t* old_state = canvas->state;
    canvas->state = old_state->next;
    juce_plutovg_state_reset(old_state);
    old_state->next = canvas->freed_state;
    canvas->freed_state = old_state;
}

void juce_plutovg_canvas_set_rgb(juce_plutovg_canvas_t* canvas, float r, float g, float b)
{
    juce_plutovg_canvas_set_rgba(canvas, r, g, b, 1.f);
}

void juce_plutovg_canvas_set_rgba(juce_plutovg_canvas_t* canvas, float r, float g, float b, float a)
{
    juce_plutovg_color_init_rgba(&canvas->state->color, r, g, b, a);
    juce_plutovg_canvas_set_paint(canvas, NULL);
}

void juce_plutovg_canvas_set_color(juce_plutovg_canvas_t* canvas, const juce_plutovg_color_t* color)
{
    juce_plutovg_canvas_set_rgba(canvas, color->r, color->g, color->b, color->a);
}

void juce_plutovg_canvas_set_linear_gradient(juce_plutovg_canvas_t* canvas, float x1, float y1, float x2, float y2, juce_plutovg_spread_method_t spread, const juce_plutovg_gradient_stop_t* stops, int nstops, const juce_plutovg_matrix_t* matrix)
{
    juce_plutovg_paint_t* paint = juce_plutovg_paint_create_linear_gradient(x1, y1, x2, y2, spread, stops, nstops, matrix);
    juce_plutovg_canvas_set_paint(canvas, paint);
    juce_plutovg_paint_destroy(paint);
}

void juce_plutovg_canvas_set_radial_gradient(juce_plutovg_canvas_t* canvas, float cx, float cy, float cr, float fx, float fy, float fr, juce_plutovg_spread_method_t spread, const juce_plutovg_gradient_stop_t* stops, int nstops, const juce_plutovg_matrix_t* matrix)
{
    juce_plutovg_paint_t* paint = juce_plutovg_paint_create_radial_gradient(cx, cy, cr, fx, fy, fr, spread, stops, nstops, matrix);
    juce_plutovg_canvas_set_paint(canvas, paint);
    juce_plutovg_paint_destroy(paint);
}

void juce_plutovg_canvas_set_texture(juce_plutovg_canvas_t* canvas, juce_plutovg_surface_t* surface, juce_plutovg_texture_type_t type, float opacity, const juce_plutovg_matrix_t* matrix)
{
    juce_plutovg_paint_t* paint = juce_plutovg_paint_create_texture(surface, type, opacity, matrix);
    juce_plutovg_canvas_set_paint(canvas, paint);
    juce_plutovg_paint_destroy(paint);
}

void juce_plutovg_canvas_set_paint(juce_plutovg_canvas_t* canvas, juce_plutovg_paint_t* paint)
{
    paint = juce_plutovg_paint_reference(paint);
    juce_plutovg_paint_destroy(canvas->state->paint);
    canvas->state->paint = paint;
}

juce_plutovg_paint_t* juce_plutovg_canvas_get_paint(const juce_plutovg_canvas_t* canvas, juce_plutovg_color_t* color)
{
    if(color)
        *color = canvas->state->color;
    return canvas->state->paint;
}

void juce_plutovg_canvas_set_font_face_cache(juce_plutovg_canvas_t* canvas, juce_plutovg_font_face_cache_t* cache)
{
    cache = juce_plutovg_font_face_cache_reference(cache);
    juce_plutovg_font_face_cache_destroy(canvas->face_cache);
    canvas->face_cache = cache;
}

juce_plutovg_font_face_cache_t* juce_plutovg_canvas_get_font_face_cache(const juce_plutovg_canvas_t* canvas)
{
    return canvas->face_cache;
}

void juce_plutovg_canvas_add_font_face(juce_plutovg_canvas_t* canvas, const char* family, bool bold, bool italic, juce_plutovg_font_face_t* face)
{
    if(canvas->face_cache == NULL)
        canvas->face_cache = juce_plutovg_font_face_cache_create();
    juce_plutovg_font_face_cache_add(canvas->face_cache, family, bold, italic, face);
}

bool juce_plutovg_canvas_add_font_file(juce_plutovg_canvas_t* canvas, const char* family, bool bold, bool italic, const char* filename, int ttcindex)
{
    if(canvas->face_cache == NULL)
        canvas->face_cache = juce_plutovg_font_face_cache_create();
    return juce_plutovg_font_face_cache_add_file(canvas->face_cache, family, bold, italic, filename, ttcindex);
}

bool juce_plutovg_canvas_select_font_face(juce_plutovg_canvas_t* canvas, const char* family, bool bold, bool italic)
{
    if(canvas->face_cache == NULL)
        return false;
    juce_plutovg_font_face_t* face = juce_plutovg_font_face_cache_get(canvas->face_cache, family, bold, italic);
    if(face == NULL)
        return false;
    juce_plutovg_canvas_set_font_face(canvas, face);
    return true;
}

void juce_plutovg_canvas_set_font(juce_plutovg_canvas_t* canvas, juce_plutovg_font_face_t* face, float size)
{
    juce_plutovg_canvas_set_font_face(canvas, face);
    juce_plutovg_canvas_set_font_size(canvas, size);
}

void juce_plutovg_canvas_set_font_face(juce_plutovg_canvas_t* canvas, juce_plutovg_font_face_t* face)
{
    face = juce_plutovg_font_face_reference(face);
    juce_plutovg_font_face_destroy(canvas->state->font_face);
    canvas->state->font_face = face;
}

juce_plutovg_font_face_t* juce_plutovg_canvas_get_font_face(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->font_face;
}

void juce_plutovg_canvas_set_font_size(juce_plutovg_canvas_t* canvas, float size)
{
    canvas->state->font_size = size;
}

float juce_plutovg_canvas_get_font_size(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->font_size;
}

void juce_plutovg_canvas_set_fill_rule(juce_plutovg_canvas_t* canvas, juce_plutovg_fill_rule_t winding)
{
    canvas->state->winding = winding;
}

juce_plutovg_fill_rule_t juce_plutovg_canvas_get_fill_rule(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->winding;
}

void juce_plutovg_canvas_set_operator(juce_plutovg_canvas_t* canvas, juce_plutovg_operator_t op)
{
    canvas->state->op = op;
}

juce_plutovg_operator_t juce_plutovg_canvas_get_operator(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->op;
}

void juce_plutovg_canvas_set_opacity(juce_plutovg_canvas_t* canvas, float opacity)
{
    canvas->state->opacity = juce_plutovg_clamp(opacity, 0.f, 1.f);
}

float juce_plutovg_canvas_get_opacity(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->opacity;
}

void juce_plutovg_canvas_set_line_width(juce_plutovg_canvas_t* canvas, float line_width)
{
    canvas->state->stroke.style.width = line_width;
}

float juce_plutovg_canvas_get_line_width(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.style.width;
}

void juce_plutovg_canvas_set_line_cap(juce_plutovg_canvas_t* canvas, juce_plutovg_line_cap_t line_cap)
{
    canvas->state->stroke.style.cap = line_cap;
}

juce_plutovg_line_cap_t juce_plutovg_canvas_get_line_cap(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.style.cap;
}

void juce_plutovg_canvas_set_line_join(juce_plutovg_canvas_t* canvas, juce_plutovg_line_join_t line_join)
{
    canvas->state->stroke.style.join = line_join;
}

juce_plutovg_line_join_t juce_plutovg_canvas_get_line_join(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.style.join;
}

void juce_plutovg_canvas_set_miter_limit(juce_plutovg_canvas_t* canvas, float miter_limit)
{
    canvas->state->stroke.style.miter_limit = miter_limit;
}

float juce_plutovg_canvas_get_miter_limit(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.style.miter_limit;
}

void juce_plutovg_canvas_set_dash(juce_plutovg_canvas_t* canvas, float offset, const float* dashes, int ndashes)
{
    juce_plutovg_canvas_set_dash_offset(canvas, offset);
    juce_plutovg_canvas_set_dash_array(canvas, dashes, ndashes);
}

void juce_plutovg_canvas_set_dash_offset(juce_plutovg_canvas_t* canvas, float offset)
{
    canvas->state->stroke.dash.offset = offset;
}

float juce_plutovg_canvas_get_dash_offset(const juce_plutovg_canvas_t* canvas)
{
    return canvas->state->stroke.dash.offset;
}

void juce_plutovg_canvas_set_dash_array(juce_plutovg_canvas_t* canvas, const float* dashes, int ndashes)
{
    juce_plutovg_array_clear(canvas->state->stroke.dash.array);
    juce_plutovg_array_append_data(canvas->state->stroke.dash.array, dashes, ndashes);
}

int juce_plutovg_canvas_get_dash_array(const juce_plutovg_canvas_t* canvas, const float** dashes)
{
    if(dashes)
        *dashes = canvas->state->stroke.dash.array.data;
    return canvas->state->stroke.dash.array.size;
}

void juce_plutovg_canvas_translate(juce_plutovg_canvas_t* canvas, float tx, float ty)
{
    juce_plutovg_matrix_translate(&canvas->state->matrix, tx, ty);
}

void juce_plutovg_canvas_scale(juce_plutovg_canvas_t* canvas, float sx, float sy)
{
    juce_plutovg_matrix_scale(&canvas->state->matrix, sx, sy);
}

void juce_plutovg_canvas_shear(juce_plutovg_canvas_t* canvas, float shx, float shy)
{
    juce_plutovg_matrix_shear(&canvas->state->matrix, shx, shy);
}

void juce_plutovg_canvas_rotate(juce_plutovg_canvas_t* canvas, float angle)
{
    juce_plutovg_matrix_rotate(&canvas->state->matrix, angle);
}

void juce_plutovg_canvas_transform(juce_plutovg_canvas_t* canvas, const juce_plutovg_matrix_t* matrix)
{
    juce_plutovg_matrix_multiply(&canvas->state->matrix, matrix, &canvas->state->matrix);
}

void juce_plutovg_canvas_reset_matrix(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_matrix_init_identity(&canvas->state->matrix);
}

void juce_plutovg_canvas_set_matrix(juce_plutovg_canvas_t* canvas, const juce_plutovg_matrix_t* matrix)
{
    canvas->state->matrix = matrix ? *matrix : JUCE_PLUTOVG_IDENTITY_MATRIX;
}

void juce_plutovg_canvas_get_matrix(const juce_plutovg_canvas_t* canvas, juce_plutovg_matrix_t* matrix)
{
    *matrix = canvas->state->matrix;
}

void juce_plutovg_canvas_map(const juce_plutovg_canvas_t* canvas, float x, float y, float* xx, float* yy)
{
    juce_plutovg_matrix_map(&canvas->state->matrix, x, y, xx, yy);
}

void juce_plutovg_canvas_map_point(const juce_plutovg_canvas_t* canvas, const juce_plutovg_point_t* src, juce_plutovg_point_t* dst)
{
    juce_plutovg_matrix_map_point(&canvas->state->matrix, src, dst);
}

void juce_plutovg_canvas_map_rect(const juce_plutovg_canvas_t* canvas, const juce_plutovg_rect_t* src, juce_plutovg_rect_t* dst)
{
    juce_plutovg_matrix_map_rect(&canvas->state->matrix, src, dst);
}

void juce_plutovg_canvas_move_to(juce_plutovg_canvas_t* canvas, float x, float y)
{
    juce_plutovg_path_move_to(canvas->path, x, y);
}

void juce_plutovg_canvas_line_to(juce_plutovg_canvas_t* canvas, float x, float y)
{
    juce_plutovg_path_line_to(canvas->path, x, y);
}

void juce_plutovg_canvas_quad_to(juce_plutovg_canvas_t* canvas, float x1, float y1, float x2, float y2)
{
    juce_plutovg_path_quad_to(canvas->path, x1, y1, x2, y2);
}

void juce_plutovg_canvas_cubic_to(juce_plutovg_canvas_t* canvas, float x1, float y1, float x2, float y2, float x3, float y3)
{
    juce_plutovg_path_cubic_to(canvas->path, x1, y1, x2, y2, x3, y3);
}

void juce_plutovg_canvas_arc_to(juce_plutovg_canvas_t* canvas, float rx, float ry, float angle, bool large_arc_flag, bool sweep_flag, float x, float y)
{
    juce_plutovg_path_arc_to(canvas->path, rx, ry, angle, large_arc_flag, sweep_flag, x, y);
}

void juce_plutovg_canvas_rect(juce_plutovg_canvas_t* canvas, float x, float y, float w, float h)
{
    juce_plutovg_path_add_rect(canvas->path, x, y, w, h);
}

void juce_plutovg_canvas_round_rect(juce_plutovg_canvas_t* canvas, float x, float y, float w, float h, float rx, float ry)
{
    juce_plutovg_path_add_round_rect(canvas->path, x, y, w, h, rx, ry);
}

void juce_plutovg_canvas_ellipse(juce_plutovg_canvas_t* canvas, float cx, float cy, float rx, float ry)
{
    juce_plutovg_path_add_ellipse(canvas->path, cx, cy, rx, ry);
}

void juce_plutovg_canvas_circle(juce_plutovg_canvas_t* canvas, float cx, float cy, float r)
{
    juce_plutovg_path_add_circle(canvas->path, cx, cy, r);
}

void juce_plutovg_canvas_arc(juce_plutovg_canvas_t* canvas, float cx, float cy, float r, float a0, float a1, bool ccw)
{
    juce_plutovg_path_add_arc(canvas->path, cx, cy, r, a0, a1, ccw);
}

void juce_plutovg_canvas_add_path(juce_plutovg_canvas_t* canvas, const juce_plutovg_path_t* path)
{
    juce_plutovg_path_add_path(canvas->path, path, NULL);
}

void juce_plutovg_canvas_new_path(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_path_reset(canvas->path);
}

void juce_plutovg_canvas_close_path(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_path_close(canvas->path);
}

void juce_plutovg_canvas_get_current_point(const juce_plutovg_canvas_t* canvas, float* x, float* y)
{
    juce_plutovg_path_get_current_point(canvas->path, x, y);
}

juce_plutovg_path_t* juce_plutovg_canvas_get_path(const juce_plutovg_canvas_t* canvas)
{
    return canvas->path;
}

bool juce_plutovg_canvas_fill_contains(juce_plutovg_canvas_t* canvas, float x, float y)
{
    juce_plutovg_rasterize(&canvas->fill_spans, canvas->path, &canvas->state->matrix, NULL, NULL, canvas->state->winding);
    return juce_plutovg_span_buffer_contains(&canvas->fill_spans, x, y);
}

bool juce_plutovg_canvas_stroke_contains(juce_plutovg_canvas_t* canvas, float x, float y)
{
    juce_plutovg_rasterize(&canvas->fill_spans, canvas->path, &canvas->state->matrix, NULL, NULL, canvas->state->winding);
    return juce_plutovg_span_buffer_contains(&canvas->fill_spans, x, y);
}

bool juce_plutovg_canvas_clip_contains(juce_plutovg_canvas_t* canvas, float x, float y)
{
    if(canvas->state->clipping) {
        return juce_plutovg_span_buffer_contains(&canvas->state->clip_spans, x, y);
    }

    float l = canvas->clip_rect.x;
    float t = canvas->clip_rect.y;
    float r = canvas->clip_rect.x + canvas->clip_rect.w;
    float b = canvas->clip_rect.y + canvas->clip_rect.h;

    return x >= l && x <= r && y >= t && y <= b;
}

void juce_plutovg_canvas_fill_extents(juce_plutovg_canvas_t *canvas, juce_plutovg_rect_t* extents)
{
    juce_plutovg_rasterize(&canvas->fill_spans, canvas->path, &canvas->state->matrix, NULL, NULL, canvas->state->winding);
    juce_plutovg_span_buffer_extents(&canvas->fill_spans, extents);
}

void juce_plutovg_canvas_stroke_extents(juce_plutovg_canvas_t *canvas, juce_plutovg_rect_t* extents)
{
    juce_plutovg_rasterize(&canvas->fill_spans, canvas->path, &canvas->state->matrix, NULL, &canvas->state->stroke, JUCE_PLUTOVG_FILL_RULE_NON_ZERO);
    juce_plutovg_span_buffer_extents(&canvas->fill_spans, extents);
}

void juce_plutovg_canvas_clip_extents(juce_plutovg_canvas_t* canvas, juce_plutovg_rect_t* extents)
{
    if(canvas->state->clipping) {
        juce_plutovg_span_buffer_extents(&canvas->state->clip_spans, extents);
    } else {
        extents->x = canvas->clip_rect.x;
        extents->y = canvas->clip_rect.y;
        extents->w = canvas->clip_rect.w;
        extents->h = canvas->clip_rect.h;
    }
}

void juce_plutovg_canvas_fill(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_canvas_fill_preserve(canvas);
    juce_plutovg_canvas_new_path(canvas);
}

void juce_plutovg_canvas_stroke(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_canvas_stroke_preserve(canvas);
    juce_plutovg_canvas_new_path(canvas);
}

void juce_plutovg_canvas_clip(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_canvas_clip_preserve(canvas);
    juce_plutovg_canvas_new_path(canvas);
}

void juce_plutovg_canvas_paint(juce_plutovg_canvas_t* canvas)
{
    if(canvas->state->clipping) {
        juce_plutovg_blend(canvas, &canvas->state->clip_spans);
    } else {
        juce_plutovg_span_buffer_init_rect(&canvas->clip_spans, 0, 0, canvas->surface->width, canvas->surface->height);
        juce_plutovg_blend(canvas, &canvas->clip_spans);
    }
}

void juce_plutovg_canvas_fill_preserve(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_rasterize(&canvas->fill_spans, canvas->path, &canvas->state->matrix, &canvas->clip_rect, NULL, canvas->state->winding);
    if(canvas->state->clipping) {
        juce_plutovg_span_buffer_intersect(&canvas->clip_spans, &canvas->fill_spans, &canvas->state->clip_spans);
        juce_plutovg_blend(canvas, &canvas->clip_spans);
    } else {
        juce_plutovg_blend(canvas, &canvas->fill_spans);
    }
}

void juce_plutovg_canvas_stroke_preserve(juce_plutovg_canvas_t* canvas)
{
    juce_plutovg_rasterize(&canvas->fill_spans, canvas->path, &canvas->state->matrix, &canvas->clip_rect, &canvas->state->stroke, JUCE_PLUTOVG_FILL_RULE_NON_ZERO);
    if(canvas->state->clipping) {
        juce_plutovg_span_buffer_intersect(&canvas->clip_spans, &canvas->fill_spans, &canvas->state->clip_spans);
        juce_plutovg_blend(canvas, &canvas->clip_spans);
    } else {
        juce_plutovg_blend(canvas, &canvas->fill_spans);
    }
}

void juce_plutovg_canvas_clip_preserve(juce_plutovg_canvas_t* canvas)
{
    if(canvas->state->clipping) {
        juce_plutovg_rasterize(&canvas->fill_spans, canvas->path, &canvas->state->matrix, &canvas->clip_rect, NULL, canvas->state->winding);
        juce_plutovg_span_buffer_intersect(&canvas->clip_spans, &canvas->fill_spans, &canvas->state->clip_spans);
        juce_plutovg_span_buffer_copy(&canvas->state->clip_spans, &canvas->clip_spans);
    } else {
        juce_plutovg_rasterize(&canvas->state->clip_spans, canvas->path, &canvas->state->matrix, &canvas->clip_rect, NULL, canvas->state->winding);
        canvas->state->clipping = true;
    }
}

void juce_plutovg_canvas_fill_rect(juce_plutovg_canvas_t* canvas, float x, float y, float w, float h)
{
    juce_plutovg_canvas_new_path(canvas);
    juce_plutovg_canvas_rect(canvas, x, y, w, h);
    juce_plutovg_canvas_fill(canvas);
}

void juce_plutovg_canvas_fill_path(juce_plutovg_canvas_t* canvas, const juce_plutovg_path_t* path)
{
    juce_plutovg_canvas_new_path(canvas);
    juce_plutovg_canvas_add_path(canvas, path);
    juce_plutovg_canvas_fill(canvas);
}

void juce_plutovg_canvas_stroke_rect(juce_plutovg_canvas_t* canvas, float x, float y, float w, float h)
{
    juce_plutovg_canvas_new_path(canvas);
    juce_plutovg_canvas_rect(canvas, x, y, w, h);
    juce_plutovg_canvas_stroke(canvas);
}

void juce_plutovg_canvas_stroke_path(juce_plutovg_canvas_t* canvas, const juce_plutovg_path_t* path)
{
    juce_plutovg_canvas_new_path(canvas);
    juce_plutovg_canvas_add_path(canvas, path);
    juce_plutovg_canvas_stroke(canvas);
}

void juce_plutovg_canvas_clip_rect(juce_plutovg_canvas_t* canvas, float x, float y, float w, float h)
{
    juce_plutovg_canvas_new_path(canvas);
    juce_plutovg_canvas_rect(canvas, x, y, w, h);
    juce_plutovg_canvas_clip(canvas);
}

void juce_plutovg_canvas_clip_path(juce_plutovg_canvas_t* canvas, const juce_plutovg_path_t* path)
{
    juce_plutovg_canvas_new_path(canvas);
    juce_plutovg_canvas_add_path(canvas, path);
    juce_plutovg_canvas_clip(canvas);
}

float juce_plutovg_canvas_add_glyph(juce_plutovg_canvas_t* canvas, juce_plutovg_codepoint_t codepoint, float x, float y)
{
    juce_plutovg_state_t* state = canvas->state;
    if(state->font_face && state->font_size > 0.f)
        return juce_plutovg_font_face_get_glyph_path(state->font_face, state->font_size, x, y, codepoint, canvas->path);
    return 0.f;
}

float juce_plutovg_canvas_add_text(juce_plutovg_canvas_t* canvas, const void* text, int length, juce_plutovg_text_encoding_t encoding, float x, float y)
{
    juce_plutovg_state_t* state = canvas->state;
    if(state->font_face == NULL || state->font_size <= 0.f)
        return 0.f;
    juce_plutovg_text_iterator_t it;
    juce_plutovg_text_iterator_init(&it, text, length, encoding);
    float advance_width = 0.f;
    while(juce_plutovg_text_iterator_has_next(&it)) {
        juce_plutovg_codepoint_t codepoint = juce_plutovg_text_iterator_next(&it);
        advance_width += juce_plutovg_font_face_get_glyph_path(state->font_face, state->font_size, x + advance_width, y, codepoint, canvas->path);
    }

    return advance_width;
}

float juce_plutovg_canvas_fill_text(juce_plutovg_canvas_t* canvas, const void* text, int length, juce_plutovg_text_encoding_t encoding, float x, float y)
{
    juce_plutovg_canvas_new_path(canvas);
    float advance_width = juce_plutovg_canvas_add_text(canvas, text, length, encoding, x, y);
    juce_plutovg_canvas_fill(canvas);
    return advance_width;
}

float juce_plutovg_canvas_stroke_text(juce_plutovg_canvas_t* canvas, const void* text, int length, juce_plutovg_text_encoding_t encoding, float x, float y)
{
    juce_plutovg_canvas_new_path(canvas);
    float advance_width = juce_plutovg_canvas_add_text(canvas, text, length, encoding, x, y);
    juce_plutovg_canvas_stroke(canvas);
    return advance_width;
}

float juce_plutovg_canvas_clip_text(juce_plutovg_canvas_t* canvas, const void* text, int length, juce_plutovg_text_encoding_t encoding, float x, float y)
{
    juce_plutovg_canvas_new_path(canvas);
    float advance_width = juce_plutovg_canvas_add_text(canvas, text, length, encoding, x, y);
    juce_plutovg_canvas_clip(canvas);
    return advance_width;
}

void juce_plutovg_canvas_font_metrics(const juce_plutovg_canvas_t* canvas, float* ascent, float* descent, float* line_gap, juce_plutovg_rect_t* extents)
{
    juce_plutovg_state_t* state = canvas->state;
    if(state->font_face && state->font_size > 0.f) {
        juce_plutovg_font_face_get_metrics(state->font_face, state->font_size, ascent, descent, line_gap, extents);
        return;
    }

    if(ascent) *ascent = 0.f;
    if(descent) *descent = 0.f;
    if(line_gap) *line_gap = 0.f;
    if(extents) {
        extents->x = 0.f;
        extents->y = 0.f;
        extents->w = 0.f;
        extents->h = 0.f;
    }
}

void juce_plutovg_canvas_glyph_metrics(juce_plutovg_canvas_t* canvas, juce_plutovg_codepoint_t codepoint, float* advance_width, float* left_side_bearing, juce_plutovg_rect_t* extents)
{
    juce_plutovg_state_t* state = canvas->state;
    if(state->font_face && state->font_size > 0.f) {
        juce_plutovg_font_face_get_glyph_metrics(state->font_face, state->font_size, codepoint, advance_width, left_side_bearing, extents);
        return;
    }

    if(advance_width) *advance_width = 0.f;
    if(left_side_bearing) *left_side_bearing = 0.f;
    if(extents) {
        extents->x = 0.f;
        extents->y = 0.f;
        extents->w = 0.f;
        extents->h = 0.f;
    }
}

float juce_plutovg_canvas_text_extents(juce_plutovg_canvas_t* canvas, const void* text, int length, juce_plutovg_text_encoding_t encoding, juce_plutovg_rect_t* extents)
{
    juce_plutovg_state_t* state = canvas->state;
    if(state->font_face && state->font_size > 0.f) {
        return juce_plutovg_font_face_text_extents(state->font_face, state->font_size, text, length, encoding, extents);
    }

    if(extents) {
        extents->x = 0.f;
        extents->y = 0.f;
        extents->w = 0.f;
        extents->h = 0.f;
    }

    return 0.f;
}
