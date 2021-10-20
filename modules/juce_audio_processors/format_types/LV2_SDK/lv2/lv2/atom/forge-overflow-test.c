/*
  Copyright 2019 David Robillard <d@drobilla.net>

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

#include "lv2/atom/atom-test-utils.c"
#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/urid/urid.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

static int
test_string_overflow(void)
{
#define MAX_CHARS 15

  static const size_t capacity = sizeof(LV2_Atom_String) + MAX_CHARS + 1;
  static const char*  str      = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  uint8_t*       buf = (uint8_t*)malloc(capacity);
  LV2_URID_Map   map = {NULL, urid_map};
  LV2_Atom_Forge forge;
  lv2_atom_forge_init(&forge, &map);

  // Check that writing increasingly long strings fails at the right point
  for (size_t count = 0; count < MAX_CHARS; ++count) {
    lv2_atom_forge_set_buffer(&forge, buf, capacity);

    const LV2_Atom_Forge_Ref ref = lv2_atom_forge_string(&forge, str, count);
    if (!ref) {
      return test_fail("Failed to write %zu byte string\n", count);
    }
  }

  // Failure writing to an exactly full forge
  if (lv2_atom_forge_string(&forge, str, MAX_CHARS + 1)) {
    return test_fail("Successfully wrote past end of buffer\n");
  }

  // Failure writing body after successfully writing header
  lv2_atom_forge_set_buffer(&forge, buf, sizeof(LV2_Atom) + 1);
  if (lv2_atom_forge_string(&forge, "AB", 2)) {
    return test_fail("Successfully wrote atom header past end\n");
  }

  free(buf);
  return 0;
}

static int
test_literal_overflow(void)
{
  static const size_t capacity = sizeof(LV2_Atom_Literal) + 2;

  uint8_t*       buf = (uint8_t*)malloc(capacity);
  LV2_URID_Map   map = {NULL, urid_map};
  LV2_Atom_Forge forge;
  lv2_atom_forge_init(&forge, &map);

  // Failure in atom header
  lv2_atom_forge_set_buffer(&forge, buf, 1);
  if (lv2_atom_forge_literal(&forge, "A", 1, 0, 0)) {
    return test_fail("Successfully wrote atom header past end\n");
  }

  // Failure in literal header
  lv2_atom_forge_set_buffer(&forge, buf, sizeof(LV2_Atom) + 1);
  if (lv2_atom_forge_literal(&forge, "A", 1, 0, 0)) {
    return test_fail("Successfully wrote literal header past end\n");
  }

  // Success (only room for one character + null terminator)
  lv2_atom_forge_set_buffer(&forge, buf, capacity);
  if (!lv2_atom_forge_literal(&forge, "A", 1, 0, 0)) {
    return test_fail("Failed to write small enough literal\n");
  }

  // Failure in body
  lv2_atom_forge_set_buffer(&forge, buf, capacity);
  if (lv2_atom_forge_literal(&forge, "AB", 2, 0, 0)) {
    return test_fail("Successfully wrote literal body past end\n");
  }

  free(buf);
  return 0;
}

static int
test_sequence_overflow(void)
{
  static const size_t size = sizeof(LV2_Atom_Sequence) + 6 * sizeof(LV2_Atom);
  LV2_URID_Map        map  = {NULL, urid_map};

  // Test over a range that fails in the sequence header and event components
  for (size_t capacity = 1; capacity < size; ++capacity) {
    uint8_t* buf = (uint8_t*)malloc(capacity);

    LV2_Atom_Forge forge;
    lv2_atom_forge_init(&forge, &map);
    lv2_atom_forge_set_buffer(&forge, buf, capacity);

    LV2_Atom_Forge_Frame frame;
    LV2_Atom_Forge_Ref   ref = lv2_atom_forge_sequence_head(&forge, &frame, 0);

    assert(capacity >= sizeof(LV2_Atom_Sequence) || !frame.ref);
    assert(capacity >= sizeof(LV2_Atom_Sequence) || !ref);
    (void)ref;

    lv2_atom_forge_frame_time(&forge, 0);
    lv2_atom_forge_int(&forge, 42);
    lv2_atom_forge_pop(&forge, &frame);

    free(buf);
  }

  return 0;
}

static int
test_vector_head_overflow(void)
{
  static const size_t size = sizeof(LV2_Atom_Vector) + 3 * sizeof(LV2_Atom);
  LV2_URID_Map        map  = {NULL, urid_map};

  // Test over a range that fails in the vector header and elements
  for (size_t capacity = 1; capacity < size; ++capacity) {
    uint8_t* buf = (uint8_t*)malloc(capacity);

    LV2_Atom_Forge forge;
    lv2_atom_forge_init(&forge, &map);
    lv2_atom_forge_set_buffer(&forge, buf, capacity);

    LV2_Atom_Forge_Frame frame;
    LV2_Atom_Forge_Ref   ref =
      lv2_atom_forge_vector_head(&forge, &frame, sizeof(int32_t), forge.Int);

    assert(capacity >= sizeof(LV2_Atom_Vector) || !frame.ref);
    assert(capacity >= sizeof(LV2_Atom_Vector) || !ref);
    (void)ref;

    lv2_atom_forge_int(&forge, 1);
    lv2_atom_forge_int(&forge, 2);
    lv2_atom_forge_int(&forge, 3);
    lv2_atom_forge_pop(&forge, &frame);

    free(buf);
  }

  return 0;
}

static int
test_vector_overflow(void)
{
  static const size_t  size  = sizeof(LV2_Atom_Vector) + 3 * sizeof(LV2_Atom);
  static const int32_t vec[] = {1, 2, 3};
  LV2_URID_Map         map   = {NULL, urid_map};

  // Test over a range that fails in the vector header and elements
  for (size_t capacity = 1; capacity < size; ++capacity) {
    uint8_t* buf = (uint8_t*)malloc(capacity);

    LV2_Atom_Forge forge;
    lv2_atom_forge_init(&forge, &map);
    lv2_atom_forge_set_buffer(&forge, buf, capacity);

    LV2_Atom_Forge_Ref ref =
      lv2_atom_forge_vector(&forge, sizeof(int32_t), forge.Int, 3, vec);

    assert(capacity >= sizeof(LV2_Atom_Vector) || !ref);
    (void)ref;

    free(buf);
  }

  return 0;
}

static int
test_tuple_overflow(void)
{
  static const size_t size = sizeof(LV2_Atom_Tuple) + 3 * sizeof(LV2_Atom);
  LV2_URID_Map        map  = {NULL, urid_map};

  // Test over a range that fails in the tuple header and elements
  for (size_t capacity = 1; capacity < size; ++capacity) {
    uint8_t* buf = (uint8_t*)malloc(capacity);

    LV2_Atom_Forge forge;
    lv2_atom_forge_init(&forge, &map);
    lv2_atom_forge_set_buffer(&forge, buf, capacity);

    LV2_Atom_Forge_Frame frame;
    LV2_Atom_Forge_Ref   ref = lv2_atom_forge_tuple(&forge, &frame);

    assert(capacity >= sizeof(LV2_Atom_Tuple) || !frame.ref);
    assert(capacity >= sizeof(LV2_Atom_Tuple) || !ref);
    (void)ref;

    lv2_atom_forge_int(&forge, 1);
    lv2_atom_forge_float(&forge, 2.0f);
    lv2_atom_forge_string(&forge, "three", 5);
    lv2_atom_forge_pop(&forge, &frame);

    free(buf);
  }

  return 0;
}

int
main(void)
{
  const int ret = test_string_overflow() || test_literal_overflow() ||
                  test_sequence_overflow() || test_vector_head_overflow() ||
                  test_vector_overflow() || test_tuple_overflow();

  free_urid_map();

  return ret;
}
