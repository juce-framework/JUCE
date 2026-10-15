/*
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Author(s): Behdad Esfahbod
 */

#include "hb.hh"

#ifdef HAVE_HARFRUST

#include "hb-shaper-impl.hh"

#include "hb-utf.hh"


/*
 * buffer
 */
extern "C" void *
_hb_harfrust_buffer_create_rs (void);

extern "C" void
_hb_harfrust_buffer_destroy_rs (void *data);


/*
 * shaper face data
 */

extern "C" void *
_hb_harfrust_shaper_face_data_create_rs (hb_face_t *face);

hb_harfrust_face_data_t *
_hb_harfrust_shaper_face_data_create (hb_face_t *face)
{
  return (hb_harfrust_face_data_t *) _hb_harfrust_shaper_face_data_create_rs (face);
}

extern "C" void
_hb_harfrust_shaper_face_data_destroy_rs (void *data);

void
_hb_harfrust_shaper_face_data_destroy (hb_harfrust_face_data_t *data)
{
  _hb_harfrust_shaper_face_data_destroy_rs (data);
}


/*
 * shaper font data
 */

extern "C" void *
_hb_harfrust_shaper_font_data_create_rs (hb_font_t *font, const void *face_data);

hb_harfrust_font_data_t *
_hb_harfrust_shaper_font_data_create (hb_font_t *font)
{
  const hb_harfrust_face_data_t *face_data = font->face->data.harfrust;
  return (hb_harfrust_font_data_t *) _hb_harfrust_shaper_font_data_create_rs (font, face_data);
}

extern "C" void
_hb_harfrust_shaper_font_data_destroy_rs (void *data);

void
_hb_harfrust_shaper_font_data_destroy (hb_harfrust_font_data_t *data)
{
  _hb_harfrust_shaper_font_data_destroy_rs (data);
}


/*
 * shape plan
 */

extern "C" void *
_hb_harfrust_shape_plan_create_rs (const void *font_data,
				   hb_script_t script,
				   hb_language_t language,
				   hb_direction_t direction,
				   const hb_feature_t *features,
				   unsigned int        num_features);

extern "C" void
_hb_harfrust_shape_plan_destroy_rs (void *data);


/*
 * shaper
 */

extern "C" hb_bool_t
_hb_harfrust_shape_rs (const void         *font_data,
		       const void         *rs_shape_plan,
		       const void         *rs_buffer,
		       hb_font_t          *font,
		       hb_buffer_t        *buffer,
		       const hb_codepoint_t *pre_context,
		       uint32_t            pre_context_len,
		       const hb_codepoint_t *post_context,
		       uint32_t            post_context_len,
		       const hb_feature_t *features,
		       unsigned int        num_features);

static hb_user_data_key_t hb_buffer_key = {0};

hb_bool_t
_hb_harfrust_shape (hb_shape_plan_t    *shape_plan,
		    hb_font_t          *font,
		    hb_buffer_t        *buffer,
		    const hb_feature_t *features,
		    unsigned int        num_features)
{
  const hb_harfrust_font_data_t *font_data = font->data.harfrust;

retry_buffer:
  void *hr_buffer = hb_buffer_get_user_data (buffer, &hb_buffer_key);
  if (unlikely (!hr_buffer))
  {
    hr_buffer = _hb_harfrust_buffer_create_rs ();
    if (unlikely (!hr_buffer))
      return false;

    if (!hb_buffer_set_user_data (buffer,
				  &hb_buffer_key,
				  hr_buffer,
				  _hb_harfrust_buffer_destroy_rs,
				  false))
    {
      _hb_harfrust_buffer_destroy_rs (hr_buffer);
      goto retry_buffer;
    }
  }

  void *hr_shape_plan = nullptr;

retry_shape_plan:
  hr_shape_plan = shape_plan->harfrust_data.get_acquire ();
  if (unlikely (!hr_shape_plan))
  {
    hr_shape_plan = _hb_harfrust_shape_plan_create_rs (font_data,
						       shape_plan->key.props.script,
						       shape_plan->key.props.language,
						       shape_plan->key.props.direction,
						       features, num_features);
    if (hr_shape_plan &&
	!shape_plan->harfrust_data.cmpexch (nullptr, hr_shape_plan))
    {
      _hb_harfrust_shape_plan_destroy_rs (hr_shape_plan);
      goto retry_shape_plan;
    }
  }

  return _hb_harfrust_shape_rs (font_data,
				hr_shape_plan,
				hr_buffer,
				font,
				buffer,
				buffer->context[0],
				buffer->context_len[0],
				buffer->context[1],
				buffer->context_len[1],
				features,
				num_features);
}


#endif
