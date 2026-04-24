/*
  Copyright 2007-2019 David Robillard <d@drobilla.net>

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

#include "filesystem.h"
#include "lilv_internal.h"

#include "lilv/lilv.h"
#include "serd/serd.h"
#include "sord/sord.h"
#include "sratom/sratom.h"
#include "zix/tree.h"

#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/core/lv2.h"
#include "lv2/presets/presets.h"
#include "lv2/state/state.h"
#include "lv2/urid/urid.h"

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USTR(s) ((const uint8_t*)(s))

typedef struct {
  void*    value; ///< Value/Object
  size_t   size;  ///< Size of value
  uint32_t key;   ///< Key/Predicate (URID)
  uint32_t type;  ///< Type of value (URID)
  uint32_t flags; ///< State flags (POD, etc)
} Property;

typedef struct {
  char*     symbol; ///< Symbol of port
  LV2_Atom* atom;   ///< Value in port
} PortValue;

typedef struct {
  char* abs; ///< Absolute path of actual file
  char* rel; ///< Abstract path (relative path in state dir)
} PathMap;

typedef struct {
  size_t    n;
  Property* props;
} PropertyArray;

struct LilvStateImpl {
  LilvNode*     plugin_uri;  ///< Plugin URI
  LilvNode*     uri;         ///< State/preset URI
  char*         dir;         ///< Save directory (if saved)
  char*         scratch_dir; ///< Directory for files created by plugin
  char*         copy_dir;    ///< Directory for snapshots of external files
  char*         link_dir;    ///< Directory for links to external files
  char*         label;       ///< State/Preset label
  ZixTree*      abs2rel;     ///< PathMap sorted by abs
  ZixTree*      rel2abs;     ///< PathMap sorted by rel
  PropertyArray props;       ///< State properties
  PropertyArray metadata;    ///< State metadata
  PortValue*    values;      ///< Port values
  uint32_t      atom_Path;   ///< atom:Path URID
  uint32_t      n_values;    ///< Number of port values
};

static int
abs_cmp(const void* a, const void* b, const void* user_data)
{
  (void)user_data;

  return strcmp(((const PathMap*)a)->abs, ((const PathMap*)b)->abs);
}

static int
rel_cmp(const void* a, const void* b, const void* user_data)
{
  (void)user_data;

  return strcmp(((const PathMap*)a)->rel, ((const PathMap*)b)->rel);
}

static int
property_cmp(const void* a, const void* b)
{
  const uint32_t a_key = ((const Property*)a)->key;
  const uint32_t b_key = ((const Property*)b)->key;

  if (a_key < b_key) {
    return -1;
  }

  if (b_key < a_key) {
    return 1;
  }

  return 0;
}

static int
value_cmp(const void* a, const void* b)
{
  return strcmp(((const PortValue*)a)->symbol, ((const PortValue*)b)->symbol);
}

static void
path_rel_free(void* ptr)
{
  free(((PathMap*)ptr)->abs);
  free(((PathMap*)ptr)->rel);
  free(ptr);
}

static PortValue*
append_port_value(LilvState*  state,
                  const char* port_symbol,
                  const void* value,
                  uint32_t    size,
                  uint32_t    type)
{
  PortValue* pv = NULL;
  if (value) {
    state->values = (PortValue*)realloc(
      state->values, (++state->n_values) * sizeof(PortValue));

    pv             = &state->values[state->n_values - 1];
    pv->symbol     = lilv_strdup(port_symbol);
    pv->atom       = (LV2_Atom*)malloc(sizeof(LV2_Atom) + size);
    pv->atom->size = size;
    pv->atom->type = type;
    memcpy(pv->atom + 1, value, size);
  }
  return pv;
}

static const char*
lilv_state_rel2abs(const LilvState* state, const char* path)
{
  ZixTreeIter*  iter = NULL;
  const PathMap key  = {NULL, (char*)path};
  if (state->rel2abs && !zix_tree_find(state->rel2abs, &key, &iter)) {
    return ((const PathMap*)zix_tree_get(iter))->abs;
  }
  return path;
}

static void
append_property(LilvState*     state,
                PropertyArray* array,
                uint32_t       key,
                const void*    value,
                size_t         size,
                uint32_t       type,
                uint32_t       flags)
{
  array->props =
    (Property*)realloc(array->props, (++array->n) * sizeof(Property));

  Property* const prop = &array->props[array->n - 1];
  if ((flags & LV2_STATE_IS_POD) || type == state->atom_Path) {
    prop->value = malloc(size);
    memcpy(prop->value, value, size);
  } else {
    prop->value = (void*)value;
  }

  prop->size  = size;
  prop->key   = key;
  prop->type  = type;
  prop->flags = flags;
}

static const Property*
find_property(const LilvState* const state, const uint32_t key)
{
  if (!state->props.props) {
    return NULL;
  }

  const Property search_key = {NULL, 0, key, 0, 0};

  return (const Property*)bsearch(&search_key,
                                  state->props.props,
                                  state->props.n,
                                  sizeof(Property),
                                  property_cmp);
}

static LV2_State_Status
store_callback(LV2_State_Handle handle,
               uint32_t         key,
               const void*      value,
               size_t           size,
               uint32_t         type,
               uint32_t         flags)
{
  LilvState* const state = (LilvState*)handle;

  if (!key) {
    return LV2_STATE_ERR_UNKNOWN; // TODO: Add status for bad arguments
  }

  if (find_property((const LilvState*)handle, key)) {
    return LV2_STATE_ERR_UNKNOWN; // TODO: Add status for duplicate keys
  }

  append_property(state, &state->props, key, value, size, type, flags);
  return LV2_STATE_SUCCESS;
}

static const void*
retrieve_callback(LV2_State_Handle handle,
                  uint32_t         key,
                  size_t*          size,
                  uint32_t*        type,
                  uint32_t*        flags)
{
  const Property* const prop = find_property((const LilvState*)handle, key);

  if (prop) {
    *size  = prop->size;
    *type  = prop->type;
    *flags = prop->flags;
    return prop->value;
  }
  return NULL;
}

static bool
path_exists(const char* path, const void* ignored)
{
  (void)ignored;

  return lilv_path_exists(path);
}

static bool
lilv_state_has_path(const char* path, const void* state)
{
  return lilv_state_rel2abs((const LilvState*)state, path) != path;
}

static char*
make_path(LV2_State_Make_Path_Handle handle, const char* path)
{
  LilvState* state = (LilvState*)handle;
  lilv_create_directories(state->dir);

  return lilv_path_join(state->dir, path);
}

static char*
abstract_path(LV2_State_Map_Path_Handle handle, const char* abs_path)
{
  LilvState*    state     = (LilvState*)handle;
  char*         path      = NULL;
  char*         real_path = lilv_path_canonical(abs_path);
  const PathMap key       = {real_path, NULL};
  ZixTreeIter*  iter      = NULL;

  if (abs_path[0] == '\0') {
    return lilv_strdup(abs_path);
  }

  if (!zix_tree_find(state->abs2rel, &key, &iter)) {
    // Already mapped path in a previous call
    PathMap* pm = (PathMap*)zix_tree_get(iter);
    free(real_path);
    return lilv_strdup(pm->rel);
  }

  if (lilv_path_is_child(real_path, state->dir)) {
    // File in state directory (loaded, or created by plugin during save)
    path = lilv_path_relative_to(real_path, state->dir);
  } else if (lilv_path_is_child(real_path, state->scratch_dir)) {
    // File created by plugin earlier
    path = lilv_path_relative_to(real_path, state->scratch_dir);
    if (state->copy_dir) {
      int st = lilv_create_directories(state->copy_dir);
      if (st) {
        LILV_ERRORF(
          "Error creating directory %s (%s)\n", state->copy_dir, strerror(st));
      }

      char* cpath = lilv_path_join(state->copy_dir, path);
      char* copy  = lilv_get_latest_copy(real_path, cpath);
      if (!copy || !lilv_file_equals(real_path, copy)) {
        // No recent enough copy, make a new one
        free(copy);
        copy = lilv_find_free_path(cpath, path_exists, NULL);
        if ((st = lilv_copy_file(real_path, copy))) {
          LILV_ERRORF("Error copying state file %s (%s)\n", copy, strerror(st));
        }
      }
      free(real_path);
      free(cpath);

      // Refer to the latest copy in plugin state
      real_path = copy;
    }
  } else if (state->link_dir) {
    // New path outside state directory, make a link
    char* const name = lilv_path_filename(real_path);

    // Find a free name in the (virtual) state directory
    path = lilv_find_free_path(name, lilv_state_has_path, state);

    free(name);
  } else {
    // No link directory, preserve absolute path
    path = lilv_strdup(abs_path);
  }

  // Add record to path mapping
  PathMap* pm = (PathMap*)malloc(sizeof(PathMap));
  pm->abs     = real_path;
  pm->rel     = lilv_strdup(path);
  zix_tree_insert(state->abs2rel, pm, NULL);
  zix_tree_insert(state->rel2abs, pm, NULL);

  return path;
}

static char*
absolute_path(LV2_State_Map_Path_Handle handle, const char* state_path)
{
  LilvState* state = (LilvState*)handle;
  char*      path  = NULL;
  if (lilv_path_is_absolute(state_path)) {
    // Absolute path, return identical path
    path = lilv_strdup(state_path);
  } else if (state->dir) {
    // Relative path inside state directory
    path = lilv_path_join(state->dir, state_path);
  } else {
    // State has not been saved, unmap
    path = lilv_strdup(lilv_state_rel2abs(state, state_path));
  }

  return path;
}

/** Return a new features array with built-in features added to `features`. */
static const LV2_Feature**
add_features(const LV2_Feature* const* features,
             const LV2_Feature*        map,
             const LV2_Feature*        make,
             const LV2_Feature*        free)
{
  size_t n_features = 0;
  for (; features && features[n_features]; ++n_features) {
  }

  const LV2_Feature** ret =
    (const LV2_Feature**)calloc(n_features + 4, sizeof(LV2_Feature*));

  if (features) {
    memcpy(ret, features, n_features * sizeof(LV2_Feature*));
  }

  size_t i = n_features;
  if (map) {
    ret[i++] = map;
  }
  if (make) {
    ret[i++] = make;
  }
  if (free) {
    ret[i++] = free;
  }

  return ret;
}

/// Return the canonical path for a directory with a trailing separator
static char*
real_dir(const char* path)
{
  char* abs_path = lilv_path_canonical(path);
  char* base     = lilv_path_join(abs_path, NULL);
  free(abs_path);
  return base;
}

static const char*
state_strerror(LV2_State_Status st)
{
  switch (st) {
  case LV2_STATE_SUCCESS:
    return "Completed successfully";
  case LV2_STATE_ERR_BAD_TYPE:
    return "Unsupported type";
  case LV2_STATE_ERR_BAD_FLAGS:
    return "Unsupported flags";
  case LV2_STATE_ERR_NO_FEATURE:
    return "Missing features";
  case LV2_STATE_ERR_NO_PROPERTY:
    return "Missing property";
  default:
    return "Unknown error";
  }
}

static void
lilv_free_path(LV2_State_Free_Path_Handle handle, char* path)
{
  (void)handle;

  lilv_free(path);
}

LilvState*
lilv_state_new_from_instance(const LilvPlugin*         plugin,
                             LilvInstance*             instance,
                             LV2_URID_Map*             map,
                             const char*               scratch_dir,
                             const char*               copy_dir,
                             const char*               link_dir,
                             const char*               save_dir,
                             LilvGetPortValueFunc      get_value,
                             void*                     user_data,
                             uint32_t                  flags,
                             const LV2_Feature* const* features)
{
  const LV2_Feature** sfeatures = NULL;
  LilvWorld* const    world     = plugin->world;
  LilvState* const    state     = (LilvState*)calloc(1, sizeof(LilvState));
  state->plugin_uri  = lilv_node_duplicate(lilv_plugin_get_uri(plugin));
  state->abs2rel     = zix_tree_new(false, abs_cmp, NULL, path_rel_free);
  state->rel2abs     = zix_tree_new(false, rel_cmp, NULL, NULL);
  state->scratch_dir = scratch_dir ? real_dir(scratch_dir) : NULL;
  state->copy_dir    = copy_dir ? real_dir(copy_dir) : NULL;
  state->link_dir    = link_dir ? real_dir(link_dir) : NULL;
  state->dir         = save_dir ? real_dir(save_dir) : NULL;
  state->atom_Path   = map->map(map->handle, LV2_ATOM__Path);

  LV2_State_Map_Path  pmap          = {state, abstract_path, absolute_path};
  LV2_Feature         pmap_feature  = {LV2_STATE__mapPath, &pmap};
  LV2_State_Make_Path pmake         = {state, make_path};
  LV2_Feature         pmake_feature = {LV2_STATE__makePath, &pmake};
  LV2_State_Free_Path pfree         = {NULL, lilv_free_path};
  LV2_Feature         pfree_feature = {LV2_STATE__freePath, &pfree};
  features = sfeatures = add_features(
    features, &pmap_feature, save_dir ? &pmake_feature : NULL, &pfree_feature);

  // Store port values
  if (get_value) {
    LilvNode* lv2_ControlPort = lilv_new_uri(world, LILV_URI_CONTROL_PORT);
    LilvNode* lv2_InputPort   = lilv_new_uri(world, LILV_URI_INPUT_PORT);
    for (uint32_t i = 0; i < plugin->num_ports; ++i) {
      const LilvPort* const port = plugin->ports[i];
      if (lilv_port_is_a(plugin, port, lv2_ControlPort) &&
          lilv_port_is_a(plugin, port, lv2_InputPort)) {
        uint32_t    size  = 0;
        uint32_t    type  = 0;
        const char* sym   = lilv_node_as_string(port->symbol);
        const void* value = get_value(sym, user_data, &size, &type);
        append_port_value(state, sym, value, size, type);
      }
    }
    lilv_node_free(lv2_ControlPort);
    lilv_node_free(lv2_InputPort);
  }

  // Store properties
  const LV2_Descriptor*      desc = instance->lv2_descriptor;
  const LV2_State_Interface* iface =
    (desc->extension_data)
      ? (const LV2_State_Interface*)desc->extension_data(LV2_STATE__interface)
      : NULL;

  if (iface) {
    LV2_State_Status st =
      iface->save(instance->lv2_handle, store_callback, state, flags, features);
    if (st) {
      LILV_ERRORF("Error saving plugin state: %s\n", state_strerror(st));
      free(state->props.props);
      state->props.props = NULL;
      state->props.n     = 0;
    } else {
      qsort(state->props.props, state->props.n, sizeof(Property), property_cmp);
    }
  }

  if (state->values) {
    qsort(state->values, state->n_values, sizeof(PortValue), value_cmp);
  }

  free(sfeatures);
  return state;
}

void
lilv_state_emit_port_values(const LilvState*     state,
                            LilvSetPortValueFunc set_value,
                            void*                user_data)
{
  for (uint32_t i = 0; i < state->n_values; ++i) {
    const PortValue* value = &state->values[i];
    const LV2_Atom*  atom  = value->atom;
    set_value(value->symbol, user_data, atom + 1, atom->size, atom->type);
  }
}

void
lilv_state_restore(const LilvState*          state,
                   LilvInstance*             instance,
                   LilvSetPortValueFunc      set_value,
                   void*                     user_data,
                   uint32_t                  flags,
                   const LV2_Feature* const* features)
{
  if (!state) {
    LILV_ERROR("lilv_state_restore() called on NULL state\n");
    return;
  }

  LV2_State_Map_Path map_path = {
    (LilvState*)state, abstract_path, absolute_path};
  LV2_Feature map_feature = {LV2_STATE__mapPath, &map_path};

  LV2_State_Free_Path free_path    = {NULL, lilv_free_path};
  LV2_Feature         free_feature = {LV2_STATE__freePath, &free_path};

  if (instance) {
    const LV2_Descriptor* desc = instance->lv2_descriptor;
    if (desc->extension_data) {
      const LV2_State_Interface* iface =
        (const LV2_State_Interface*)desc->extension_data(LV2_STATE__interface);

      if (iface && iface->restore) {
        const LV2_Feature** sfeatures =
          add_features(features, &map_feature, NULL, &free_feature);

        iface->restore(instance->lv2_handle,
                       retrieve_callback,
                       (LV2_State_Handle)state,
                       flags,
                       sfeatures);

        free(sfeatures);
      }
    }
  }

  if (set_value) {
    lilv_state_emit_port_values(state, set_value, user_data);
  }
}

static void
set_state_dir_from_model(LilvState* state, const SordNode* graph)
{
  if (!state->dir && graph) {
    const char* uri  = (const char*)sord_node_get_string(graph);
    char*       path = lilv_file_uri_parse(uri, NULL);

    state->dir = lilv_path_join(path, NULL);
    free(path);
  }
  assert(!state->dir || lilv_path_is_absolute(state->dir));
}

static LilvState*
new_state_from_model(LilvWorld*      world,
                     LV2_URID_Map*   map,
                     SordModel*      model,
                     const SordNode* node,
                     const char*     dir)
{
  // Check that we know at least something about this state subject
  if (!sord_ask(model, node, 0, 0, 0)) {
    return NULL;
  }

  // Allocate state
  LilvState* const state = (LilvState*)calloc(1, sizeof(LilvState));
  state->dir             = lilv_path_join(dir, NULL);
  state->atom_Path       = map->map(map->handle, LV2_ATOM__Path);
  state->uri             = lilv_node_new_from_node(world, node);

  // Get the plugin URI this state applies to
  SordIter* i = sord_search(model, node, world->uris.lv2_appliesTo, 0, 0);
  if (i) {
    const SordNode* object = sord_iter_get_node(i, SORD_OBJECT);
    const SordNode* graph  = sord_iter_get_node(i, SORD_GRAPH);
    state->plugin_uri      = lilv_node_new_from_node(world, object);
    set_state_dir_from_model(state, graph);
    sord_iter_free(i);
  } else if (sord_ask(
               model, node, world->uris.rdf_a, world->uris.lv2_Plugin, 0)) {
    // Loading plugin description as state (default state)
    state->plugin_uri = lilv_node_new_from_node(world, node);
  } else {
    LILV_ERRORF("State %s missing lv2:appliesTo property\n",
                sord_node_get_string(node));
  }

  // Get the state label
  i = sord_search(model, node, world->uris.rdfs_label, NULL, NULL);
  if (i) {
    const SordNode* object = sord_iter_get_node(i, SORD_OBJECT);
    const SordNode* graph  = sord_iter_get_node(i, SORD_GRAPH);
    state->label = lilv_strdup((const char*)sord_node_get_string(object));
    set_state_dir_from_model(state, graph);
    sord_iter_free(i);
  }

  Sratom*        sratom = sratom_new(map);
  SerdChunk      chunk  = {NULL, 0};
  LV2_Atom_Forge forge;
  lv2_atom_forge_init(&forge, map);
  lv2_atom_forge_set_sink(
    &forge, sratom_forge_sink, sratom_forge_deref, &chunk);

  // Get port values
  SordIter* ports = sord_search(model, node, world->uris.lv2_port, 0, 0);
  FOREACH_MATCH (ports) {
    const SordNode* port = sord_iter_get_node(ports, SORD_OBJECT);

    SordNode* label  = sord_get(model, port, world->uris.rdfs_label, 0, 0);
    SordNode* symbol = sord_get(model, port, world->uris.lv2_symbol, 0, 0);
    SordNode* value  = sord_get(model, port, world->uris.pset_value, 0, 0);
    if (!value) {
      value = sord_get(model, port, world->uris.lv2_default, 0, 0);
    }
    if (!symbol) {
      LILV_ERRORF("State `%s' port missing symbol.\n",
                  sord_node_get_string(node));
    } else if (value) {
      chunk.len = 0;
      sratom_read(sratom, &forge, world->world, model, value);
      const LV2_Atom* atom = (const LV2_Atom*)chunk.buf;

      append_port_value(state,
                        (const char*)sord_node_get_string(symbol),
                        LV2_ATOM_BODY_CONST(atom),
                        atom->size,
                        atom->type);

      if (label) {
        lilv_state_set_label(state, (const char*)sord_node_get_string(label));
      }
    }
    sord_node_free(world->world, value);
    sord_node_free(world->world, symbol);
    sord_node_free(world->world, label);
  }
  sord_iter_free(ports);

  // Get properties
  SordNode* statep     = sord_new_uri(world->world, USTR(LV2_STATE__state));
  SordNode* state_node = sord_get(model, node, statep, NULL, NULL);
  if (state_node) {
    SordIter* props = sord_search(model, state_node, 0, 0, 0);
    FOREACH_MATCH (props) {
      const SordNode* p   = sord_iter_get_node(props, SORD_PREDICATE);
      const SordNode* o   = sord_iter_get_node(props, SORD_OBJECT);
      const char*     key = (const char*)sord_node_get_string(p);

      chunk.len = 0;
      lv2_atom_forge_set_sink(
        &forge, sratom_forge_sink, sratom_forge_deref, &chunk);

      sratom_read(sratom, &forge, world->world, model, o);
      const LV2_Atom* atom  = (const LV2_Atom*)chunk.buf;
      uint32_t        flags = LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE;
      Property        prop  = {NULL, 0, 0, 0, flags};

      prop.key   = map->map(map->handle, key);
      prop.type  = atom->type;
      prop.size  = atom->size;
      prop.value = malloc(atom->size);
      memcpy(prop.value, LV2_ATOM_BODY_CONST(atom), atom->size);
      if (atom->type == forge.Path) {
        prop.flags = LV2_STATE_IS_POD;
      }

      if (prop.value) {
        state->props.props = (Property*)realloc(
          state->props.props, (++state->props.n) * sizeof(Property));
        state->props.props[state->props.n - 1] = prop;
      }
    }
    sord_iter_free(props);
  }
  sord_node_free(world->world, state_node);
  sord_node_free(world->world, statep);

  serd_free((void*)chunk.buf);
  sratom_free(sratom);

  if (state->props.props) {
    qsort(state->props.props, state->props.n, sizeof(Property), property_cmp);
  }
  if (state->values) {
    qsort(state->values, state->n_values, sizeof(PortValue), value_cmp);
  }

  return state;
}

LilvState*
lilv_state_new_from_world(LilvWorld*      world,
                          LV2_URID_Map*   map,
                          const LilvNode* node)
{
  if (!lilv_node_is_uri(node) && !lilv_node_is_blank(node)) {
    LILV_ERRORF("Subject `%s' is not a URI or blank node.\n",
                lilv_node_as_string(node));
    return NULL;
  }

  return new_state_from_model(world, map, world->model, node->node, NULL);
}

LilvState*
lilv_state_new_from_file(LilvWorld*      world,
                         LV2_URID_Map*   map,
                         const LilvNode* subject,
                         const char*     path)
{
  if (subject && !lilv_node_is_uri(subject) && !lilv_node_is_blank(subject)) {
    LILV_ERRORF("Subject `%s' is not a URI or blank node.\n",
                lilv_node_as_string(subject));
    return NULL;
  }

  uint8_t*    abs_path = (uint8_t*)lilv_path_absolute(path);
  SerdNode    node     = serd_node_new_file_uri(abs_path, NULL, NULL, true);
  SerdEnv*    env      = serd_env_new(&node);
  SordModel*  model    = sord_new(world->world, SORD_SPO, false);
  SerdReader* reader   = sord_new_reader(model, env, SERD_TURTLE, NULL);

  serd_reader_read_file(reader, node.buf);

  SordNode* subject_node =
    (subject) ? subject->node
              : sord_node_from_serd_node(world->world, env, &node, NULL, NULL);

  char*      dirname   = lilv_path_parent(path);
  char*      real_path = lilv_path_canonical(dirname);
  char*      dir_path  = lilv_path_join(real_path, NULL);
  LilvState* state =
    new_state_from_model(world, map, model, subject_node, dir_path);
  free(dir_path);
  free(real_path);
  free(dirname);

  serd_node_free(&node);
  free(abs_path);
  serd_reader_free(reader);
  sord_free(model);
  serd_env_free(env);
  return state;
}

static void
set_prefixes(SerdEnv* env)
{
#define SET_PSET(e, p, u) serd_env_set_prefix_from_strings(e, p, u)
  SET_PSET(env, USTR("atom"), USTR(LV2_ATOM_PREFIX));
  SET_PSET(env, USTR("lv2"), USTR(LV2_CORE_PREFIX));
  SET_PSET(env, USTR("pset"), USTR(LV2_PRESETS_PREFIX));
  SET_PSET(env, USTR("rdf"), USTR(LILV_NS_RDF));
  SET_PSET(env, USTR("rdfs"), USTR(LILV_NS_RDFS));
  SET_PSET(env, USTR("state"), USTR(LV2_STATE_PREFIX));
  SET_PSET(env, USTR("xsd"), USTR(LILV_NS_XSD));
}

LilvState*
lilv_state_new_from_string(LilvWorld* world, LV2_URID_Map* map, const char* str)
{
  if (!str) {
    return NULL;
  }

  SerdNode    base   = SERD_NODE_NULL;
  SerdEnv*    env    = serd_env_new(&base);
  SordModel*  model  = sord_new(world->world, SORD_SPO | SORD_OPS, false);
  SerdReader* reader = sord_new_reader(model, env, SERD_TURTLE, NULL);

  set_prefixes(env);
  serd_reader_read_string(reader, USTR(str));

  SordNode* o = sord_new_uri(world->world, USTR(LV2_PRESETS__Preset));
  SordNode* s = sord_get(model, NULL, world->uris.rdf_a, o, NULL);

  LilvState* state = new_state_from_model(world, map, model, s, NULL);

  sord_node_free(world->world, s);
  sord_node_free(world->world, o);
  serd_reader_free(reader);
  sord_free(model);
  serd_env_free(env);

  return state;
}

static SerdWriter*
ttl_writer(SerdSink sink, void* stream, const SerdNode* base, SerdEnv** new_env)
{
  SerdURI base_uri = SERD_URI_NULL;
  if (base && base->buf) {
    serd_uri_parse(base->buf, &base_uri);
  }

  SerdEnv* env = *new_env ? *new_env : serd_env_new(base);
  set_prefixes(env);

  SerdWriter* writer =
    serd_writer_new(SERD_TURTLE,
                    (SerdStyle)(SERD_STYLE_RESOLVED | SERD_STYLE_ABBREVIATED |
                                SERD_STYLE_CURIED),
                    env,
                    &base_uri,
                    sink,
                    stream);

  if (!*new_env) {
    *new_env = env;
  }

  return writer;
}

static SerdWriter*
ttl_file_writer(FILE* fd, const SerdNode* node, SerdEnv** env)
{
  SerdWriter* writer = ttl_writer(serd_file_sink, fd, node, env);

  fseek(fd, 0, SEEK_END);
  if (ftell(fd) == 0) {
    serd_env_foreach(*env, (SerdPrefixSink)serd_writer_set_prefix, writer);
  } else {
    fprintf(fd, "\n");
  }

  return writer;
}

static void
add_to_model(SordWorld*     world,
             SerdEnv*       env,
             SordModel*     model,
             const SerdNode s,
             const SerdNode p,
             const SerdNode o)
{
  SordNode* ss = sord_node_from_serd_node(world, env, &s, NULL, NULL);
  SordNode* sp = sord_node_from_serd_node(world, env, &p, NULL, NULL);
  SordNode* so = sord_node_from_serd_node(world, env, &o, NULL, NULL);

  SordQuad quad = {ss, sp, so, NULL};
  sord_add(model, quad);

  sord_node_free(world, ss);
  sord_node_free(world, sp);
  sord_node_free(world, so);
}

static void
remove_manifest_entry(SordWorld* world, SordModel* model, const char* subject)
{
  SordNode* s = sord_new_uri(world, USTR(subject));
  SordIter* i = sord_search(model, s, NULL, NULL, NULL);
  while (!sord_iter_end(i)) {
    sord_erase(model, i);
  }
  sord_iter_free(i);
  sord_node_free(world, s);
}

static int
write_manifest(LilvWorld*      world,
               SerdEnv*        env,
               SordModel*      model,
               const SerdNode* file_uri)
{
  (void)world;

  char* const path = (char*)serd_file_uri_parse(file_uri->buf, NULL);
  FILE* const wfd  = fopen(path, "w");
  if (!wfd) {
    LILV_ERRORF("Failed to open %s for writing (%s)\n", path, strerror(errno));

    serd_free(path);
    return 1;
  }

  SerdWriter* writer = ttl_file_writer(wfd, file_uri, &env);
  sord_write(model, writer, NULL);
  serd_writer_free(writer);
  fclose(wfd);
  serd_free(path);
  return 0;
}

static int
add_state_to_manifest(LilvWorld*      lworld,
                      const LilvNode* plugin_uri,
                      const char*     manifest_path,
                      const char*     state_uri,
                      const char*     state_path)
{
  SordWorld* world    = lworld->world;
  SerdNode   manifest = serd_node_new_file_uri(USTR(manifest_path), 0, 0, 1);
  SerdNode   file     = serd_node_new_file_uri(USTR(state_path), 0, 0, 1);
  SerdEnv*   env      = serd_env_new(&manifest);
  SordModel* model    = sord_new(world, SORD_SPO, false);

  if (lilv_path_exists(manifest_path)) {
    // Read manifest into model
    SerdReader* reader = sord_new_reader(model, env, SERD_TURTLE, NULL);
    SerdStatus  st     = serd_reader_read_file(reader, manifest.buf);
    if (st) {
      LILV_WARNF("Failed to read manifest (%s)\n", serd_strerror(st));
    }
    serd_reader_free(reader);
  }

  // Choose state URI (use file URI if not given)
  if (!state_uri) {
    state_uri = (const char*)file.buf;
  }

  // Remove any existing manifest entries for this state
  remove_manifest_entry(world, model, state_uri);

  // Add manifest entry for this state to model
  SerdNode s = serd_node_from_string(SERD_URI, USTR(state_uri));

  // <state> a pset:Preset
  add_to_model(world,
               env,
               model,
               s,
               serd_node_from_string(SERD_URI, USTR(LILV_NS_RDF "type")),
               serd_node_from_string(SERD_URI, USTR(LV2_PRESETS__Preset)));

  // <state> a pset:Preset
  add_to_model(world,
               env,
               model,
               s,
               serd_node_from_string(SERD_URI, USTR(LILV_NS_RDF "type")),
               serd_node_from_string(SERD_URI, USTR(LV2_PRESETS__Preset)));

  // <state> rdfs:seeAlso <file>
  add_to_model(world,
               env,
               model,
               s,
               serd_node_from_string(SERD_URI, USTR(LILV_NS_RDFS "seeAlso")),
               file);

  // <state> lv2:appliesTo <plugin>
  add_to_model(
    world,
    env,
    model,
    s,
    serd_node_from_string(SERD_URI, USTR(LV2_CORE__appliesTo)),
    serd_node_from_string(SERD_URI, USTR(lilv_node_as_string(plugin_uri))));

  /* Re-open manifest for locked writing.  We need to do this because it may
     need to be truncated, and the file can only be open once on Windows. */

  FILE* wfd = fopen(manifest_path, "wb");
  int   r   = 0;
  if (!wfd) {
    LILV_ERRORF(
      "Failed to open %s for writing (%s)\n", manifest_path, strerror(errno));
    r = 1;
  }

  SerdWriter* writer = ttl_file_writer(wfd, &manifest, &env);
  lilv_flock(wfd, true, true);
  sord_write(model, writer, NULL);
  lilv_flock(wfd, false, true);
  serd_writer_free(writer);
  fclose(wfd);

  sord_free(model);
  serd_node_free(&file);
  serd_node_free(&manifest);
  serd_env_free(env);

  return r;
}

static bool
link_exists(const char* path, const void* data)
{
  const char* target = (const char*)data;
  if (!lilv_path_exists(path)) {
    return false;
  }
  char* real_path = lilv_path_canonical(path);
  bool  matches   = !strcmp(real_path, target);
  free(real_path);
  return !matches;
}

static int
maybe_symlink(const char* oldpath, const char* newpath)
{
  if (link_exists(newpath, oldpath)) {
    return 0;
  }

  const int st = lilv_symlink(oldpath, newpath);
  if (st) {
    LILV_ERRORF(
      "Failed to link %s => %s (%s)\n", newpath, oldpath, strerror(errno));
  }

  return st;
}

static void
write_property_array(const LilvState*     state,
                     const PropertyArray* array,
                     Sratom*              sratom,
                     uint32_t             flags,
                     const SerdNode*      subject,
                     LV2_URID_Unmap*      unmap,
                     const char*          dir)
{
  for (uint32_t i = 0; i < array->n; ++i) {
    Property*   prop = &array->props[i];
    const char* key  = unmap->unmap(unmap->handle, prop->key);

    const SerdNode p = serd_node_from_string(SERD_URI, USTR(key));
    if (prop->type == state->atom_Path && !dir) {
      const char* path     = (const char*)prop->value;
      const char* abs_path = lilv_state_rel2abs(state, path);
      LILV_WARNF("Writing absolute path %s\n", abs_path);
      sratom_write(sratom,
                   unmap,
                   flags,
                   subject,
                   &p,
                   prop->type,
                   strlen(abs_path) + 1,
                   abs_path);
    } else if (prop->flags & LV2_STATE_IS_POD ||
               prop->type == state->atom_Path) {
      sratom_write(
        sratom, unmap, flags, subject, &p, prop->type, prop->size, prop->value);
    } else {
      LILV_WARNF("Lost non-POD property <%s> on save\n", key);
    }
  }
}

static int
lilv_state_write(LilvWorld*       world,
                 LV2_URID_Map*    map,
                 LV2_URID_Unmap*  unmap,
                 const LilvState* state,
                 SerdWriter*      writer,
                 const char*      uri,
                 const char*      dir)
{
  (void)world;

  SerdNode lv2_appliesTo =
    serd_node_from_string(SERD_CURIE, USTR("lv2:appliesTo"));

  const SerdNode* plugin_uri = sord_node_to_serd_node(state->plugin_uri->node);

  SerdNode subject = serd_node_from_string(SERD_URI, USTR(uri ? uri : ""));

  // <subject> a pset:Preset
  SerdNode p = serd_node_from_string(SERD_URI, USTR(LILV_NS_RDF "type"));

  SerdNode o = serd_node_from_string(SERD_URI, USTR(LV2_PRESETS__Preset));
  serd_writer_write_statement(writer, 0, NULL, &subject, &p, &o, NULL, NULL);

  // <subject> lv2:appliesTo <http://example.org/plugin>
  serd_writer_write_statement(
    writer, 0, NULL, &subject, &lv2_appliesTo, plugin_uri, NULL, NULL);

  // <subject> rdfs:label label
  if (state->label) {
    p = serd_node_from_string(SERD_URI, USTR(LILV_NS_RDFS "label"));
    o = serd_node_from_string(SERD_LITERAL, USTR(state->label));
    serd_writer_write_statement(writer, 0, NULL, &subject, &p, &o, NULL, NULL);
  }

  SerdEnv*        env  = serd_writer_get_env(writer);
  const SerdNode* base = serd_env_get_base_uri(env, NULL);

  Sratom* sratom = sratom_new(map);
  sratom_set_sink(sratom,
                  (const char*)base->buf,
                  (SerdStatementSink)serd_writer_write_statement,
                  (SerdEndSink)serd_writer_end_anon,
                  writer);

  // Write metadata
  sratom_set_pretty_numbers(sratom, false); // Use precise types
  write_property_array(
    state, &state->metadata, sratom, 0, &subject, unmap, dir);

  // Write port values
  sratom_set_pretty_numbers(sratom, true); // Use pretty numbers
  for (uint32_t i = 0; i < state->n_values; ++i) {
    PortValue* const value = &state->values[i];

    const SerdNode port =
      serd_node_from_string(SERD_BLANK, USTR(value->symbol));

    // <> lv2:port _:symbol
    p = serd_node_from_string(SERD_URI, USTR(LV2_CORE__port));
    serd_writer_write_statement(
      writer, SERD_ANON_O_BEGIN, NULL, &subject, &p, &port, NULL, NULL);

    // _:symbol lv2:symbol "symbol"
    p = serd_node_from_string(SERD_URI, USTR(LV2_CORE__symbol));
    o = serd_node_from_string(SERD_LITERAL, USTR(value->symbol));
    serd_writer_write_statement(
      writer, SERD_ANON_CONT, NULL, &port, &p, &o, NULL, NULL);

    // _:symbol pset:value value
    p = serd_node_from_string(SERD_URI, USTR(LV2_PRESETS__value));
    sratom_write(sratom,
                 unmap,
                 SERD_ANON_CONT,
                 &port,
                 &p,
                 value->atom->type,
                 value->atom->size,
                 value->atom + 1);

    serd_writer_end_anon(writer, &port);
  }

  // Write properties
  const SerdNode body = serd_node_from_string(SERD_BLANK, USTR("body"));
  if (state->props.n > 0) {
    p = serd_node_from_string(SERD_URI, USTR(LV2_STATE__state));
    serd_writer_write_statement(
      writer, SERD_ANON_O_BEGIN, NULL, &subject, &p, &body, NULL, NULL);
  }
  sratom_set_pretty_numbers(sratom, false); // Use precise types
  write_property_array(
    state, &state->props, sratom, SERD_ANON_CONT, &body, unmap, dir);

  if (state->props.n > 0) {
    serd_writer_end_anon(writer, &body);
  }

  sratom_free(sratom);
  return 0;
}

static void
lilv_state_make_links(const LilvState* state, const char* dir)
{
  // Create symlinks to files
  for (ZixTreeIter* i = zix_tree_begin(state->abs2rel);
       i != zix_tree_end(state->abs2rel);
       i = zix_tree_iter_next(i)) {
    const PathMap* pm = (const PathMap*)zix_tree_get(i);

    char* path = lilv_path_absolute_child(pm->rel, dir);
    if (lilv_path_is_child(pm->abs, state->copy_dir) &&
        strcmp(state->copy_dir, dir)) {
      // Link directly to snapshot in the copy directory
      maybe_symlink(pm->abs, path);
    } else if (!lilv_path_is_child(pm->abs, dir)) {
      const char* link_dir = state->link_dir ? state->link_dir : dir;
      char*       pat      = lilv_path_absolute_child(pm->rel, link_dir);
      if (!strcmp(dir, link_dir)) {
        // Link directory is save directory, make link at exact path
        remove(pat);
        maybe_symlink(pm->abs, pat);
      } else {
        // Make a link in the link directory to external file
        char* lpath = lilv_find_free_path(pat, link_exists, pm->abs);
        if (!lilv_path_exists(lpath)) {
          if (lilv_symlink(pm->abs, lpath)) {
            LILV_ERRORF("Failed to link %s => %s (%s)\n",
                        pm->abs,
                        lpath,
                        strerror(errno));
          }
        }

        // Make a link in the save directory to the external link
        char* target = lilv_path_relative_to(lpath, dir);
        maybe_symlink(lpath, path);
        free(target);
        free(lpath);
      }
      free(pat);
    }
    free(path);
  }
}

int
lilv_state_save(LilvWorld*       world,
                LV2_URID_Map*    map,
                LV2_URID_Unmap*  unmap,
                const LilvState* state,
                const char*      uri,
                const char*      dir,
                const char*      filename)
{
  if (!filename || !dir || lilv_create_directories(dir)) {
    return 1;
  }

  char*       abs_dir = real_dir(dir);
  char* const path    = lilv_path_join(abs_dir, filename);
  FILE*       fd      = fopen(path, "w");
  if (!fd) {
    LILV_ERRORF("Failed to open %s (%s)\n", path, strerror(errno));
    free(abs_dir);
    free(path);
    return 4;
  }

  // Create symlinks to files if necessary
  lilv_state_make_links(state, abs_dir);

  // Write state to Turtle file
  SerdNode    file = serd_node_new_file_uri(USTR(path), NULL, NULL, true);
  SerdNode    node = uri ? serd_node_from_string(SERD_URI, USTR(uri)) : file;
  SerdEnv*    env  = NULL;
  SerdWriter* ttl  = ttl_file_writer(fd, &file, &env);
  int         ret =
    lilv_state_write(world, map, unmap, state, ttl, (const char*)node.buf, dir);

  // Set saved dir and uri (FIXME: const violation)
  free(state->dir);
  lilv_node_free(state->uri);
  ((LilvState*)state)->dir = lilv_strdup(abs_dir);
  ((LilvState*)state)->uri = lilv_new_uri(world, (const char*)node.buf);

  serd_node_free(&file);
  serd_writer_free(ttl);
  serd_env_free(env);
  fclose(fd);

  // Add entry to manifest
  if (!ret) {
    char* const manifest = lilv_path_join(abs_dir, "manifest.ttl");

    ret = add_state_to_manifest(world, state->plugin_uri, manifest, uri, path);

    free(manifest);
  }

  free(abs_dir);
  free(path);
  return ret;
}

char*
lilv_state_to_string(LilvWorld*       world,
                     LV2_URID_Map*    map,
                     LV2_URID_Unmap*  unmap,
                     const LilvState* state,
                     const char*      uri,
                     const char*      base_uri)
{
  if (!uri) {
    LILV_ERROR("Attempt to serialise state with no URI\n");
    return NULL;
  }

  SerdChunk   chunk  = {NULL, 0};
  SerdEnv*    env    = NULL;
  SerdNode    base   = serd_node_from_string(SERD_URI, USTR(base_uri));
  SerdWriter* writer = ttl_writer(serd_chunk_sink, &chunk, &base, &env);

  lilv_state_write(world, map, unmap, state, writer, uri, NULL);

  serd_writer_free(writer);
  serd_env_free(env);
  char* str    = (char*)serd_chunk_sink_finish(&chunk);
  char* result = lilv_strdup(str);
  serd_free(str);
  return result;
}

static void
try_unlink(const char* state_dir, const char* path)
{
  if (!strncmp(state_dir, path, strlen(state_dir))) {
    if (lilv_path_exists(path) && lilv_remove(path)) {
      LILV_ERRORF("Failed to remove %s (%s)\n", path, strerror(errno));
    }
  }
}

static char*
get_canonical_path(const LilvNode* const node)
{
  char* const path      = lilv_node_get_path(node, NULL);
  char* const real_path = lilv_path_canonical(path);

  free(path);
  return real_path;
}

int
lilv_state_delete(LilvWorld* world, const LilvState* state)
{
  if (!state->dir) {
    LILV_ERROR("Attempt to delete unsaved state\n");
    return -1;
  }

  LilvNode*  bundle        = lilv_new_file_uri(world, NULL, state->dir);
  LilvNode*  manifest      = lilv_world_get_manifest_uri(world, bundle);
  char*      manifest_path = get_canonical_path(manifest);
  const bool has_manifest  = lilv_path_exists(manifest_path);
  SordModel* model         = sord_new(world->world, SORD_SPO, false);

  if (has_manifest) {
    // Read manifest into temporary local model
    SerdEnv*    env = serd_env_new(sord_node_to_serd_node(manifest->node));
    SerdReader* ttl = sord_new_reader(model, env, SERD_TURTLE, NULL);
    serd_reader_read_file(ttl, USTR(manifest_path));
    serd_reader_free(ttl);
    serd_env_free(env);
  }

  if (state->uri) {
    SordNode* file =
      sord_get(model, state->uri->node, world->uris.rdfs_seeAlso, NULL, NULL);
    if (file) {
      // Remove state file
      const uint8_t* uri       = sord_node_get_string(file);
      char*          path      = (char*)serd_file_uri_parse(uri, NULL);
      char*          real_path = lilv_path_canonical(path);
      if (path) {
        try_unlink(state->dir, real_path);
      }
      serd_free(real_path);
      serd_free(path);
    }

    // Remove any existing manifest entries for this state
    const char* state_uri_str = lilv_node_as_string(state->uri);
    remove_manifest_entry(world->world, model, state_uri_str);
    remove_manifest_entry(world->world, world->model, state_uri_str);
  }

  // Drop bundle from model
  lilv_world_unload_bundle(world, bundle);

  if (sord_num_quads(model) == 0) {
    // Manifest is empty, attempt to remove bundle entirely
    if (has_manifest) {
      try_unlink(state->dir, manifest_path);
    }

    // Remove all known files from state bundle
    if (state->abs2rel) {
      // State created from instance, get paths from map
      for (ZixTreeIter* i = zix_tree_begin(state->abs2rel);
           i != zix_tree_end(state->abs2rel);
           i = zix_tree_iter_next(i)) {
        const PathMap* pm   = (const PathMap*)zix_tree_get(i);
        char*          path = lilv_path_join(state->dir, pm->rel);
        try_unlink(state->dir, path);
        free(path);
      }
    } else {
      // State loaded from model, get paths from loaded properties
      for (uint32_t i = 0; i < state->props.n; ++i) {
        const Property* const p = &state->props.props[i];
        if (p->type == state->atom_Path) {
          try_unlink(state->dir, (const char*)p->value);
        }
      }
    }

    if (lilv_remove(state->dir)) {
      LILV_ERRORF(
        "Failed to remove directory %s (%s)\n", state->dir, strerror(errno));
    }
  } else {
    // Still something in the manifest, update and reload bundle
    const SerdNode* manifest_node = sord_node_to_serd_node(manifest->node);
    SerdEnv*        env           = serd_env_new(manifest_node);

    write_manifest(world, env, model, manifest_node);
    lilv_world_load_bundle(world, bundle);
    serd_env_free(env);
  }

  sord_free(model);
  lilv_free(manifest_path);
  lilv_node_free(manifest);
  lilv_node_free(bundle);

  return 0;
}

static void
free_property_array(LilvState* state, PropertyArray* array)
{
  for (uint32_t i = 0; i < array->n; ++i) {
    Property* prop = &array->props[i];
    if ((prop->flags & LV2_STATE_IS_POD) || prop->type == state->atom_Path) {
      free(prop->value);
    }
  }
  free(array->props);
}

void
lilv_state_free(LilvState* state)
{
  if (state) {
    free_property_array(state, &state->props);
    free_property_array(state, &state->metadata);
    for (uint32_t i = 0; i < state->n_values; ++i) {
      free(state->values[i].atom);
      free(state->values[i].symbol);
    }
    lilv_node_free(state->plugin_uri);
    lilv_node_free(state->uri);
    zix_tree_free(state->abs2rel);
    zix_tree_free(state->rel2abs);
    free(state->values);
    free(state->label);
    free(state->dir);
    free(state->scratch_dir);
    free(state->copy_dir);
    free(state->link_dir);
    free(state);
  }
}

bool
lilv_state_equals(const LilvState* a, const LilvState* b)
{
  if (!lilv_node_equals(a->plugin_uri, b->plugin_uri) ||
      (a->label && !b->label) || (b->label && !a->label) ||
      (a->label && b->label && strcmp(a->label, b->label)) ||
      a->props.n != b->props.n || a->n_values != b->n_values) {
    return false;
  }

  for (uint32_t i = 0; i < a->n_values; ++i) {
    PortValue* const av = &a->values[i];
    PortValue* const bv = &b->values[i];
    if (av->atom->size != bv->atom->size || av->atom->type != bv->atom->type ||
        strcmp(av->symbol, bv->symbol) ||
        memcmp(av->atom + 1, bv->atom + 1, av->atom->size)) {
      return false;
    }
  }

  for (uint32_t i = 0; i < a->props.n; ++i) {
    Property* const ap = &a->props.props[i];
    Property* const bp = &b->props.props[i];
    if (ap->key != bp->key || ap->type != bp->type || ap->flags != bp->flags) {
      return false;
    }

    if (ap->type == a->atom_Path) {
      if (!lilv_file_equals(lilv_state_rel2abs(a, (char*)ap->value),
                            lilv_state_rel2abs(b, (char*)bp->value))) {
        return false;
      }
    } else if (ap->size != bp->size || memcmp(ap->value, bp->value, ap->size)) {
      return false;
    }
  }

  return true;
}

unsigned
lilv_state_get_num_properties(const LilvState* state)
{
  return state->props.n;
}

const LilvNode*
lilv_state_get_plugin_uri(const LilvState* state)
{
  return state->plugin_uri;
}

const LilvNode*
lilv_state_get_uri(const LilvState* state)
{
  return state->uri;
}

const char*
lilv_state_get_label(const LilvState* state)
{
  return state->label;
}

void
lilv_state_set_label(LilvState* state, const char* label)
{
  const size_t len = strlen(label);
  state->label     = (char*)realloc(state->label, len + 1);
  memcpy(state->label, label, len + 1);
}

int
lilv_state_set_metadata(LilvState*  state,
                        uint32_t    key,
                        const void* value,
                        size_t      size,
                        uint32_t    type,
                        uint32_t    flags)
{
  append_property(state, &state->metadata, key, value, size, type, flags);
  return LV2_STATE_SUCCESS;
}
