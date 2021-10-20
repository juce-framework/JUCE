/*
  Copyright 2011-2020 David Robillard <d@drobilla.net>

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

#include "zix/btree.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// #define ZIX_BTREE_DEBUG 1
// #define ZIX_BTREE_SORTED_CHECK 1

#ifndef ZIX_BTREE_PAGE_SIZE
#  define ZIX_BTREE_PAGE_SIZE 4096
#endif

#define ZIX_BTREE_NODE_SPACE (ZIX_BTREE_PAGE_SIZE - 2 * sizeof(uint16_t))
#define ZIX_BTREE_LEAF_VALS ((ZIX_BTREE_NODE_SPACE / sizeof(void*)) - 1)
#define ZIX_BTREE_INODE_VALS (ZIX_BTREE_LEAF_VALS / 2)

struct ZixBTreeImpl {
  ZixBTreeNode*  root;
  ZixDestroyFunc destroy;
  ZixComparator  cmp;
  const void*    cmp_data;
  size_t         size;
  unsigned       height; ///< Number of levels, i.e. root only has height 1
};

struct ZixBTreeNodeImpl {
  uint16_t is_leaf;
  uint16_t n_vals;
  // On 64-bit we rely on some padding here to get page-sized nodes
  union {
    struct {
      void* vals[ZIX_BTREE_LEAF_VALS];
    } leaf;

    struct {
      void*         vals[ZIX_BTREE_INODE_VALS];
      ZixBTreeNode* children[ZIX_BTREE_INODE_VALS + 1];
    } inode;
  } data;
};

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112l) || \
  (defined(__cplusplus) && __cplusplus >= 201103L)
static_assert(sizeof(ZixBTreeNode) == ZIX_BTREE_PAGE_SIZE, "");
#endif

typedef struct {
  ZixBTreeNode* node;
  unsigned      index;
} ZixBTreeIterFrame;

struct ZixBTreeIterImpl {
  unsigned          n_levels; ///< Maximum depth of stack
  unsigned          level;    ///< Current level in stack
  ZixBTreeIterFrame stack[];  ///< Position stack
};

#ifdef ZIX_BTREE_DEBUG

static void
print_node(const ZixBTreeNode* n, const char* prefix)
{
  printf("%s[", prefix);
  for (uint16_t v = 0; v < n->n_vals; ++v) {
    printf(" %lu", (uintptr_t)n->vals[v]);
  }
  printf(" ]\n");
}

static void
print_tree(const ZixBTreeNode* parent, const ZixBTreeNode* node, int level)
{
  if (node) {
    if (!parent) {
      printf("TREE {\n");
    }
    for (int i = 0; i < level + 1; ++i) {
      printf("  ");
    }
    print_node(node, "");
    if (!node->is_leaf) {
      for (uint16_t i = 0; i < node->n_vals + 1; ++i) {
        print_tree(node, node->data.inode.children[i], level + 1);
      }
    }
    if (!parent) {
      printf("}\n");
    }
  }
}

#endif // ZIX_BTREE_DEBUG

static ZixBTreeNode*
zix_btree_node_new(const bool leaf)
{
#if !((defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112l) || \
      (defined(__cplusplus) && __cplusplus >= 201103L))
  assert(sizeof(ZixBTreeNode) == ZIX_BTREE_PAGE_SIZE);
#endif

  ZixBTreeNode* node = (ZixBTreeNode*)malloc(sizeof(ZixBTreeNode));
  if (node) {
    node->is_leaf = leaf;
    node->n_vals  = 0;
  }
  return node;
}

static void*
zix_btree_value(const ZixBTreeNode* const node, const unsigned i)
{
  assert(i < node->n_vals);
  return node->is_leaf ? node->data.leaf.vals[i] : node->data.inode.vals[i];
}

static ZixBTreeNode*
zix_btree_child(const ZixBTreeNode* const node, const unsigned i)
{
  assert(!node->is_leaf);
  assert(i <= ZIX_BTREE_INODE_VALS);
  return node->data.inode.children[i];
}

ZixBTree*
zix_btree_new(const ZixComparator  cmp,
              const void* const    cmp_data,
              const ZixDestroyFunc destroy)
{
  ZixBTree* t = (ZixBTree*)malloc(sizeof(ZixBTree));
  if (t) {
    t->root     = zix_btree_node_new(true);
    t->destroy  = destroy;
    t->cmp      = cmp;
    t->cmp_data = cmp_data;
    t->size     = 0;
    t->height   = 1;
    if (!t->root) {
      free(t);
      return NULL;
    }
  }
  return t;
}

static void
zix_btree_free_rec(ZixBTree* const t, ZixBTreeNode* const n)
{
  if (n) {
    if (n->is_leaf) {
      if (t->destroy) {
        for (uint16_t i = 0; i < n->n_vals; ++i) {
          t->destroy(n->data.leaf.vals[i]);
        }
      }
    } else {
      if (t->destroy) {
        for (uint16_t i = 0; i < n->n_vals; ++i) {
          t->destroy(n->data.inode.vals[i]);
        }
      }

      for (uint16_t i = 0; i < n->n_vals + 1; ++i) {
        zix_btree_free_rec(t, zix_btree_child(n, i));
      }
    }

    free(n);
  }
}

void
zix_btree_free(ZixBTree* const t)
{
  if (t) {
    zix_btree_free_rec(t, t->root);
    free(t);
  }
}

size_t
zix_btree_size(const ZixBTree* const t)
{
  return t->size;
}

static uint16_t
zix_btree_max_vals(const ZixBTreeNode* const node)
{
  return node->is_leaf ? ZIX_BTREE_LEAF_VALS : ZIX_BTREE_INODE_VALS;
}

static uint16_t
zix_btree_min_vals(const ZixBTreeNode* const node)
{
  return (uint16_t)(((zix_btree_max_vals(node) + 1U) / 2U) - 1U);
}

/** Shift pointers in `array` of length `n` right starting at `i`. */
static void
zix_btree_ainsert(void** const   array,
                  const unsigned n,
                  const unsigned i,
                  void* const    e)
{
  memmove(array + i + 1, array + i, (n - i) * sizeof(e));
  array[i] = e;
}

/** Erase element `i` in `array` of length `n` and return erased element. */
static void*
zix_btree_aerase(void** const array, const unsigned n, const unsigned i)
{
  void* const ret = array[i];
  memmove(array + i, array + i + 1, (n - i) * sizeof(ret));
  return ret;
}

/** Split lhs, the i'th child of `n`, into two nodes. */
static ZixBTreeNode*
zix_btree_split_child(ZixBTreeNode* const n,
                      const unsigned      i,
                      ZixBTreeNode* const lhs)
{
  assert(lhs->n_vals == zix_btree_max_vals(lhs));
  assert(n->n_vals < ZIX_BTREE_INODE_VALS);
  assert(i < n->n_vals + 1U);
  assert(zix_btree_child(n, i) == lhs);

  const uint16_t max_n_vals = zix_btree_max_vals(lhs);
  ZixBTreeNode*  rhs        = zix_btree_node_new(lhs->is_leaf);
  if (!rhs) {
    return NULL;
  }

  // LHS and RHS get roughly half, less the middle value which moves up
  lhs->n_vals = max_n_vals / 2U;
  rhs->n_vals = (uint16_t)(max_n_vals - lhs->n_vals - 1);

  if (lhs->is_leaf) {
    // Copy large half from LHS to new RHS node
    memcpy(rhs->data.leaf.vals,
           lhs->data.leaf.vals + lhs->n_vals + 1,
           rhs->n_vals * sizeof(void*));

    // Move middle value up to parent
    zix_btree_ainsert(
      n->data.inode.vals, n->n_vals, i, lhs->data.leaf.vals[lhs->n_vals]);
  } else {
    // Copy large half from LHS to new RHS node
    memcpy(rhs->data.inode.vals,
           lhs->data.inode.vals + lhs->n_vals + 1,
           rhs->n_vals * sizeof(void*));
    memcpy(rhs->data.inode.children,
           lhs->data.inode.children + lhs->n_vals + 1,
           (rhs->n_vals + 1U) * sizeof(ZixBTreeNode*));

    // Move middle value up to parent
    zix_btree_ainsert(
      n->data.inode.vals, n->n_vals, i, lhs->data.inode.vals[lhs->n_vals]);
  }

  // Insert new RHS node in parent at position i
  zix_btree_ainsert((void**)n->data.inode.children, ++n->n_vals, i + 1U, rhs);

  return rhs;
}

#ifdef ZIX_BTREE_SORTED_CHECK
/** Check that `n` is sorted with respect to search key `e`. */
static bool
zix_btree_node_is_sorted_with_respect_to(const ZixBTree* const     t,
                                         const ZixBTreeNode* const n,
                                         const void* const         e)
{
  if (n->n_vals <= 1) {
    return true;
  }

  int cmp = t->cmp(zix_btree_value(n, 0), e, t->cmp_data);
  for (uint16_t i = 1; i < n->n_vals; ++i) {
    const int next_cmp = t->cmp(zix_btree_value(n, i), e, t->cmp_data);
    if ((cmp >= 0 && next_cmp < 0) || (cmp > 0 && next_cmp <= 0)) {
      return false;
    }
    cmp = next_cmp;
  }

  return true;
}
#endif

/** Find the first value in `n` that is not less than `e` (lower bound). */
static unsigned
zix_btree_node_find(const ZixBTree* const     t,
                    const ZixBTreeNode* const n,
                    const void* const         e,
                    bool* const               equal)
{
#ifdef ZIX_BTREE_SORTED_CHECK
  assert(zix_btree_node_is_sorted_with_respect_to(t, n, e));
#endif

  unsigned first = 0U;
  unsigned len   = n->n_vals;
  while (len > 0) {
    const unsigned half = len >> 1U;
    const unsigned i    = first + half;
    const int      cmp  = t->cmp(zix_btree_value(n, i), e, t->cmp_data);
    if (cmp == 0) {
      *equal = true;
      len    = half; // Keep searching for wildcard matches
    } else if (cmp < 0) {
      const unsigned chop = half + 1U;
      first += chop;
      len -= chop;
    } else {
      len = half;
    }
  }

  assert(!*equal || t->cmp(zix_btree_value(n, first), e, t->cmp_data) == 0);
  return first;
}

ZixStatus
zix_btree_insert(ZixBTree* const t, void* const e)
{
  ZixBTreeNode* parent = NULL;    // Parent of n
  ZixBTreeNode* n      = t->root; // Current node
  unsigned      i      = 0;       // Index of n in parent
  while (n) {
    if (n->n_vals == zix_btree_max_vals(n)) {
      // Node is full, split to ensure there is space for a leaf split
      if (!parent) {
        // Root is full, grow tree upwards
        if (!(parent = zix_btree_node_new(false))) {
          return ZIX_STATUS_NO_MEM;
        }
        t->root                        = parent;
        parent->data.inode.children[0] = n;
        ++t->height;
      }

      ZixBTreeNode* const rhs = zix_btree_split_child(parent, i, n);
      if (!rhs) {
        return ZIX_STATUS_NO_MEM;
      }

      const int cmp = t->cmp(parent->data.inode.vals[i], e, t->cmp_data);
      if (cmp == 0) {
        return ZIX_STATUS_EXISTS;
      }

      if (cmp < 0) {
        // Move to new RHS
        n = rhs;
        ++i;
      }
    }

    assert(!parent || zix_btree_child(parent, i) == n);

    bool equal = false;
    i          = zix_btree_node_find(t, n, e, &equal);
    if (equal) {
      return ZIX_STATUS_EXISTS;
    }

    if (!n->is_leaf) {
      // Descend to child node left of value
      parent = n;
      n      = zix_btree_child(n, i);
    } else {
      // Insert into internal node
      zix_btree_ainsert(n->data.leaf.vals, n->n_vals++, i, e);
      break;
    }
  }

  ++t->size;

  return ZIX_STATUS_SUCCESS;
}

static ZixBTreeIter*
zix_btree_iter_new(const ZixBTree* const t)
{
  const size_t s = t->height * sizeof(ZixBTreeIterFrame);

  ZixBTreeIter* i = (ZixBTreeIter*)calloc(1, sizeof(ZixBTreeIter) + s);
  if (i) {
    i->n_levels = t->height;
  }
  return i;
}

static void
zix_btree_iter_set_frame(ZixBTreeIter* const ti,
                         ZixBTreeNode* const n,
                         const unsigned      i)
{
  if (ti) {
    ti->stack[ti->level].node  = n;
    ti->stack[ti->level].index = i;
  }
}

static bool
zix_btree_node_is_minimal(ZixBTreeNode* const n)
{
  assert(n->n_vals >= zix_btree_min_vals(n));
  return n->n_vals == zix_btree_min_vals(n);
}

/** Enlarge left child by stealing a value from its right sibling. */
static ZixBTreeNode*
zix_btree_rotate_left(ZixBTreeNode* const parent, const unsigned i)
{
  ZixBTreeNode* const lhs = zix_btree_child(parent, i);
  ZixBTreeNode* const rhs = zix_btree_child(parent, i + 1);

  assert(lhs->is_leaf == rhs->is_leaf);

  if (lhs->is_leaf) {
    // Move parent value to end of LHS
    lhs->data.leaf.vals[lhs->n_vals++] = parent->data.inode.vals[i];

    // Move first value in RHS to parent
    parent->data.inode.vals[i] =
      zix_btree_aerase(rhs->data.leaf.vals, rhs->n_vals, 0);
  } else {
    // Move parent value to end of LHS
    lhs->data.inode.vals[lhs->n_vals++] = parent->data.inode.vals[i];

    // Move first value in RHS to parent
    parent->data.inode.vals[i] =
      zix_btree_aerase(rhs->data.inode.vals, rhs->n_vals, 0);

    // Move first child pointer from RHS to end of LHS
    lhs->data.inode.children[lhs->n_vals] = (ZixBTreeNode*)zix_btree_aerase(
      (void**)rhs->data.inode.children, rhs->n_vals, 0);
  }

  --rhs->n_vals;

  return lhs;
}

/** Enlarge right child by stealing a value from its left sibling. */
static ZixBTreeNode*
zix_btree_rotate_right(ZixBTreeNode* const parent, const unsigned i)
{
  ZixBTreeNode* const lhs = zix_btree_child(parent, i - 1);
  ZixBTreeNode* const rhs = zix_btree_child(parent, i);

  assert(lhs->is_leaf == rhs->is_leaf);

  if (lhs->is_leaf) {
    // Prepend parent value to RHS
    zix_btree_ainsert(
      rhs->data.leaf.vals, rhs->n_vals++, 0, parent->data.inode.vals[i - 1]);

    // Move last value from LHS to parent
    parent->data.inode.vals[i - 1] = lhs->data.leaf.vals[--lhs->n_vals];
  } else {
    // Prepend parent value to RHS
    zix_btree_ainsert(
      rhs->data.inode.vals, rhs->n_vals++, 0, parent->data.inode.vals[i - 1]);

    // Move last child pointer from LHS and prepend to RHS
    zix_btree_ainsert((void**)rhs->data.inode.children,
                      rhs->n_vals,
                      0,
                      lhs->data.inode.children[lhs->n_vals]);

    // Move last value from LHS to parent
    parent->data.inode.vals[i - 1] = lhs->data.inode.vals[--lhs->n_vals];
  }

  return rhs;
}

/** Move n[i] down, merge the left and right child, return the merged node. */
static ZixBTreeNode*
zix_btree_merge(ZixBTree* const t, ZixBTreeNode* const n, const unsigned i)
{
  ZixBTreeNode* const lhs = zix_btree_child(n, i);
  ZixBTreeNode* const rhs = zix_btree_child(n, i + 1);

  assert(lhs->is_leaf == rhs->is_leaf);
  assert(zix_btree_node_is_minimal(lhs));
  assert(lhs->n_vals + rhs->n_vals < zix_btree_max_vals(lhs));

  // Move parent value to end of LHS
  if (lhs->is_leaf) {
    lhs->data.leaf.vals[lhs->n_vals++] =
      zix_btree_aerase(n->data.inode.vals, n->n_vals, i);
  } else {
    lhs->data.inode.vals[lhs->n_vals++] =
      zix_btree_aerase(n->data.inode.vals, n->n_vals, i);
  }

  // Erase corresponding child pointer (to RHS) in parent
  zix_btree_aerase((void**)n->data.inode.children, n->n_vals, i + 1U);

  // Add everything from RHS to end of LHS
  if (lhs->is_leaf) {
    memcpy(lhs->data.leaf.vals + lhs->n_vals,
           rhs->data.leaf.vals,
           rhs->n_vals * sizeof(void*));
  } else {
    memcpy(lhs->data.inode.vals + lhs->n_vals,
           rhs->data.inode.vals,
           rhs->n_vals * sizeof(void*));
    memcpy(lhs->data.inode.children + lhs->n_vals,
           rhs->data.inode.children,
           (rhs->n_vals + 1U) * sizeof(void*));
  }

  lhs->n_vals = (uint16_t)(lhs->n_vals + rhs->n_vals);

  if (--n->n_vals == 0) {
    // Root is now empty, replace it with its only child
    assert(n == t->root);
    t->root = lhs;
    free(n);
  }

  free(rhs);
  return lhs;
}

/** Remove and return the min value from the subtree rooted at `n`. */
static void*
zix_btree_remove_min(ZixBTree* const t, ZixBTreeNode* n)
{
  while (!n->is_leaf) {
    if (zix_btree_node_is_minimal(zix_btree_child(n, 0))) {
      // Leftmost child is minimal, must expand
      if (!zix_btree_node_is_minimal(zix_btree_child(n, 1))) {
        // Child's right sibling has at least one key to steal
        n = zix_btree_rotate_left(n, 0);
      } else {
        // Both child and right sibling are minimal, merge
        n = zix_btree_merge(t, n, 0);
      }
    } else {
      n = zix_btree_child(n, 0);
    }
  }

  return zix_btree_aerase(n->data.leaf.vals, --n->n_vals, 0);
}

/** Remove and return the max value from the subtree rooted at `n`. */
static void*
zix_btree_remove_max(ZixBTree* const t, ZixBTreeNode* n)
{
  while (!n->is_leaf) {
    if (zix_btree_node_is_minimal(zix_btree_child(n, n->n_vals))) {
      // Leftmost child is minimal, must expand
      if (!zix_btree_node_is_minimal(zix_btree_child(n, n->n_vals - 1))) {
        // Child's left sibling has at least one key to steal
        n = zix_btree_rotate_right(n, n->n_vals);
      } else {
        // Both child and left sibling are minimal, merge
        n = zix_btree_merge(t, n, n->n_vals - 1U);
      }
    } else {
      n = zix_btree_child(n, n->n_vals);
    }
  }

  return n->data.leaf.vals[--n->n_vals];
}

ZixStatus
zix_btree_remove(ZixBTree* const      t,
                 const void* const    e,
                 void** const         out,
                 ZixBTreeIter** const next)
{
  ZixBTreeNode* n         = t->root;
  ZixBTreeIter* ti        = NULL;
  const bool    user_iter = next && *next;
  if (next) {
    if (!*next && !(*next = zix_btree_iter_new(t))) {
      return ZIX_STATUS_NO_MEM;
    }
    ti        = *next;
    ti->level = 0;
  }

  while (true) {
    /* To remove in a single walk down, the tree is adjusted along the way
       so that the current node always has at least one more value than the
       minimum required in general. Thus, there is always room to remove
       without adjusting on the way back up. */
    assert(n == t->root || !zix_btree_node_is_minimal(n));

    bool           equal = false;
    const unsigned i     = zix_btree_node_find(t, n, e, &equal);
    zix_btree_iter_set_frame(ti, n, i);
    if (n->is_leaf) {
      if (equal) {
        // Found in leaf node
        *out = zix_btree_aerase(n->data.leaf.vals, --n->n_vals, i);
        if (ti && i == n->n_vals) {
          if (i == 0) {
            ti->level         = 0;
            ti->stack[0].node = NULL;
          } else {
            --ti->stack[ti->level].index;
            zix_btree_iter_increment(ti);
          }
        }
        --t->size;
        return ZIX_STATUS_SUCCESS;
      }

      // Not found in leaf node, or tree
      if (ti && !user_iter) {
        zix_btree_iter_free(ti);
        *next = NULL;
      }

      return ZIX_STATUS_NOT_FOUND;
    }

    if (equal) {
      // Found in internal node
      ZixBTreeNode* const lhs    = zix_btree_child(n, i);
      ZixBTreeNode* const rhs    = zix_btree_child(n, i + 1);
      const size_t        l_size = lhs->n_vals;
      const size_t        r_size = rhs->n_vals;
      if (zix_btree_node_is_minimal(lhs) && zix_btree_node_is_minimal(rhs)) {
        // Both preceding and succeeding child are minimal
        n = zix_btree_merge(t, n, i);
      } else if (l_size >= r_size) {
        // Left child can remove without merge
        assert(!zix_btree_node_is_minimal(lhs));
        *out                  = n->data.inode.vals[i];
        n->data.inode.vals[i] = zix_btree_remove_max(t, lhs);
        --t->size;
        return ZIX_STATUS_SUCCESS;
      } else {
        // Right child can remove without merge
        assert(!zix_btree_node_is_minimal(rhs));
        *out                  = n->data.inode.vals[i];
        n->data.inode.vals[i] = zix_btree_remove_min(t, rhs);
        --t->size;
        return ZIX_STATUS_SUCCESS;
      }
    } else {
      // Not found in internal node, key is in/under children[i]
      if (zix_btree_node_is_minimal(zix_btree_child(n, i))) {
        if (i > 0 && !zix_btree_node_is_minimal(zix_btree_child(n, i - 1))) {
          // Steal a key from child's left sibling
          n = zix_btree_rotate_right(n, i);
        } else if (i < n->n_vals &&
                   !zix_btree_node_is_minimal(zix_btree_child(n, i + 1))) {
          // Steal a key from child's right sibling
          n = zix_btree_rotate_left(n, i);
        } else if (n == t->root && n->n_vals == 1) {
          // Root has two children, both minimal, delete it
          assert(i == 0 || i == 1);
          const uint16_t counts[2] = {zix_btree_child(n, 0)->n_vals,
                                      zix_btree_child(n, 1)->n_vals};

          n = zix_btree_merge(t, n, 0);
          if (ti) {
            ti->stack[ti->level].node  = n;
            ti->stack[ti->level].index = counts[i];
          }
        } else {
          // Both child's siblings are minimal, merge them
          if (i < n->n_vals) {
            n = zix_btree_merge(t, n, i);
          } else {
            n = zix_btree_merge(t, n, i - 1U);
            if (ti) {
              --ti->stack[ti->level].index;
            }
          }
        }
      } else {
        n = zix_btree_child(n, i);
      }
    }
    if (ti) {
      ++ti->level;
    }
  }

  assert(false); // Not reached
  return ZIX_STATUS_ERROR;
}

ZixStatus
zix_btree_find(const ZixBTree* const t,
               const void* const     e,
               ZixBTreeIter** const  ti)
{
  ZixBTreeNode* n = t->root;
  if (!(*ti = zix_btree_iter_new(t))) {
    return ZIX_STATUS_NO_MEM;
  }

  while (n) {
    bool           equal = false;
    const unsigned i     = zix_btree_node_find(t, n, e, &equal);

    zix_btree_iter_set_frame(*ti, n, i);

    if (equal) {
      return ZIX_STATUS_SUCCESS;
    }

    if (n->is_leaf) {
      break;
    }

    ++(*ti)->level;
    n = zix_btree_child(n, i);
  }

  zix_btree_iter_free(*ti);
  *ti = NULL;
  return ZIX_STATUS_NOT_FOUND;
}

ZixStatus
zix_btree_lower_bound(const ZixBTree* const t,
                      const void* const     e,
                      ZixBTreeIter** const  ti)
{
  if (!t) {
    *ti = NULL;
    return ZIX_STATUS_BAD_ARG;
  }

  if (!t->root) {
    *ti = NULL;
    return ZIX_STATUS_SUCCESS;
  }

  ZixBTreeNode* n           = t->root;
  bool          found       = false;
  unsigned      found_level = 0;
  if (!(*ti = zix_btree_iter_new(t))) {
    return ZIX_STATUS_NO_MEM;
  }

  while (n) {
    bool           equal = false;
    const unsigned i     = zix_btree_node_find(t, n, e, &equal);

    zix_btree_iter_set_frame(*ti, n, i);

    if (equal) {
      found_level = (*ti)->level;
      found       = true;
    }

    if (n->is_leaf) {
      break;
    }

    ++(*ti)->level;
    n = zix_btree_child(n, i);
    assert(n);
  }

  const ZixBTreeIterFrame* const frame = &(*ti)->stack[(*ti)->level];
  assert(frame->node);
  if (frame->index == frame->node->n_vals) {
    if (found) {
      // Found on a previous level but went too far
      (*ti)->level = found_level;
    } else {
      // Reached end (key is greater than everything in tree)
      (*ti)->level         = 0;
      (*ti)->stack[0].node = NULL;
    }
  }

  return ZIX_STATUS_SUCCESS;
}

void*
zix_btree_get(const ZixBTreeIter* const ti)
{
  const ZixBTreeIterFrame* const frame = &ti->stack[ti->level];
  assert(frame->node);
  assert(frame->index < frame->node->n_vals);
  return zix_btree_value(frame->node, frame->index);
}

ZixBTreeIter*
zix_btree_begin(const ZixBTree* const t)
{
  ZixBTreeIter* const i = zix_btree_iter_new(t);
  if (!i) {
    return NULL;
  }

  if (t->size == 0) {
    i->level         = 0;
    i->stack[0].node = NULL;
  } else {
    ZixBTreeNode* n   = t->root;
    i->stack[0].node  = n;
    i->stack[0].index = 0;
    while (!n->is_leaf) {
      n = zix_btree_child(n, 0);
      ++i->level;
      i->stack[i->level].node  = n;
      i->stack[i->level].index = 0;
    }
  }

  return i;
}

ZixBTreeIter*
zix_btree_end(const ZixBTree* const t)
{
  return zix_btree_iter_new(t);
}

ZixBTreeIter*
zix_btree_iter_copy(const ZixBTreeIter* const i)
{
  if (!i) {
    return NULL;
  }

  const size_t  s = i->n_levels * sizeof(ZixBTreeIterFrame);
  ZixBTreeIter* j = (ZixBTreeIter*)calloc(1, sizeof(ZixBTreeIter) + s);
  if (j) {
    memcpy(j, i, sizeof(ZixBTreeIter) + s);
  }

  return j;
}

bool
zix_btree_iter_is_end(const ZixBTreeIter* const i)
{
  return !i || (i->level == 0 && i->stack[0].node == NULL);
}

bool
zix_btree_iter_equals(const ZixBTreeIter* const lhs,
                      const ZixBTreeIter* const rhs)
{
  if (zix_btree_iter_is_end(lhs) && zix_btree_iter_is_end(rhs)) {
    return true;
  }

  if (zix_btree_iter_is_end(lhs) || zix_btree_iter_is_end(rhs) ||
      lhs->level != rhs->level) {
    return false;
  }

  return !memcmp(lhs,
                 rhs,
                 sizeof(ZixBTreeIter) +
                   (lhs->level + 1) * sizeof(ZixBTreeIterFrame));
}

void
zix_btree_iter_increment(ZixBTreeIter* const i)
{
  ZixBTreeIterFrame* f = &i->stack[i->level];
  if (f->node->is_leaf) {
    // Leaf, move right
    assert(f->index < f->node->n_vals);
    if (++f->index == f->node->n_vals) {
      // Reached end of leaf, move up
      f = &i->stack[i->level];
      while (i->level > 0 && f->index == f->node->n_vals) {
        f = &i->stack[--i->level];
        assert(f->index <= f->node->n_vals);
      }

      if (f->index == f->node->n_vals) {
        // Reached end of tree
        assert(i->level == 0);
        f->node  = NULL;
        f->index = 0;
      }
    }
  } else {
    // Internal node, move down to next child
    assert(f->index < f->node->n_vals);
    ZixBTreeNode* child = zix_btree_child(f->node, ++f->index);

    f        = &i->stack[++i->level];
    f->node  = child;
    f->index = 0;

    // Move down and left until we hit a leaf
    while (!f->node->is_leaf) {
      child    = zix_btree_child(f->node, 0);
      f        = &i->stack[++i->level];
      f->node  = child;
      f->index = 0;
    }
  }
}

void
zix_btree_iter_free(ZixBTreeIter* const i)
{
  free(i);
}
