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

/// @file serd.h API for Serd, a lightweight RDF syntax library

#ifndef SERD_SERD_H
#define SERD_SERD_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(_WIN32) && !defined(SERD_STATIC) && defined(SERD_INTERNAL)
#  define SERD_API __declspec(dllexport)
#elif defined(_WIN32) && !defined(SERD_STATIC)
#  define SERD_API __declspec(dllimport)
#elif defined(__GNUC__)
#  define SERD_API __attribute__((visibility("default")))
#else
#  define SERD_API
#endif

#ifdef __GNUC__
#  define SERD_PURE_FUNC __attribute__((pure))
#  define SERD_CONST_FUNC __attribute__((const))
#else
#  define SERD_PURE_FUNC
#  define SERD_CONST_FUNC
#endif

#if defined(__clang__) && __clang_major__ >= 7
#  define SERD_NONNULL _Nonnull
#  define SERD_NULLABLE _Nullable
#  define SERD_ALLOCATED _Null_unspecified
#else
#  define SERD_NONNULL
#  define SERD_NULLABLE
#  define SERD_ALLOCATED
#endif

#define SERD_PURE_API \
  SERD_API            \
  SERD_PURE_FUNC

#define SERD_CONST_API \
  SERD_API             \
  SERD_CONST_FUNC

#ifndef SERD_DISABLE_DEPRECATED
#  if defined(__clang__) && __clang_major__ >= 7
#    define SERD_DEPRECATED_BY(rep) __attribute__((deprecated("", rep)))
#  elif defined(__GNUC__) && __GNUC__ > 4
#    define SERD_DEPRECATED_BY(rep) __attribute__((deprecated("Use " rep)))
#  elif defined(__GNUC__)
#    define SERD_DEPRECATED_BY(rep) __attribute__((deprecated))
#  else
#    define SERD_DEPRECATED_BY(rep)
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#  if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#  endif
#endif

/**
   @defgroup serd Serd C API
   This is the complete public C API of serd.
   @{
*/

/// Lexical environment for relative URIs or CURIEs (base URI and namespaces)
typedef struct SerdEnvImpl SerdEnv;

/// Streaming parser that reads a text stream and writes to a statement sink
typedef struct SerdReaderImpl SerdReader;

/// Streaming serialiser that writes a text stream as statements are pushed
typedef struct SerdWriterImpl SerdWriter;

/// Return status code
typedef enum {
  SERD_SUCCESS,        ///< No error
  SERD_FAILURE,        ///< Non-fatal failure
  SERD_ERR_UNKNOWN,    ///< Unknown error
  SERD_ERR_BAD_SYNTAX, ///< Invalid syntax
  SERD_ERR_BAD_ARG,    ///< Invalid argument
  SERD_ERR_NOT_FOUND,  ///< Not found
  SERD_ERR_ID_CLASH,   ///< Encountered clashing blank node IDs
  SERD_ERR_BAD_CURIE,  ///< Invalid CURIE (e.g. prefix does not exist)
  SERD_ERR_INTERNAL    ///< Unexpected internal error (should not happen)
} SerdStatus;

/// RDF syntax type
typedef enum {
  SERD_TURTLE   = 1, ///< Terse triples http://www.w3.org/TR/turtle
  SERD_NTRIPLES = 2, ///< Line-based triples http://www.w3.org/TR/n-triples/
  SERD_NQUADS   = 3, ///< Line-based quads http://www.w3.org/TR/n-quads/
  SERD_TRIG     = 4  ///< Terse quads http://www.w3.org/TR/trig/
} SerdSyntax;

/// Flags indicating inline abbreviation information for a statement
typedef enum {
  SERD_EMPTY_S      = 1u << 1u, ///< Empty blank node subject
  SERD_EMPTY_O      = 1u << 2u, ///< Empty blank node object
  SERD_ANON_S_BEGIN = 1u << 3u, ///< Start of anonymous subject
  SERD_ANON_O_BEGIN = 1u << 4u, ///< Start of anonymous object
  SERD_ANON_CONT    = 1u << 5u, ///< Continuation of anonymous node
  SERD_LIST_S_BEGIN = 1u << 6u, ///< Start of list subject
  SERD_LIST_O_BEGIN = 1u << 7u, ///< Start of list object
  SERD_LIST_CONT    = 1u << 8u  ///< Continuation of list
} SerdStatementFlag;

/// Bitwise OR of SerdStatementFlag values
typedef uint32_t SerdStatementFlags;

/**
   Type of a node.

   An RDF node, in the abstract sense, can be either a resource, literal, or a
   blank.  This type is more precise, because syntactically there are two ways
   to refer to a resource (by URI or CURIE).

   There are also two ways to refer to a blank node in syntax (by ID or
   anonymously), but this is handled by statement flags rather than distinct
   node types.
*/
typedef enum {
  /**
     The type of a nonexistent node.

     This type is useful as a sentinel, but is never emitted by the reader.
  */
  SERD_NOTHING = 0,

  /**
     Literal value.

     A literal optionally has either a language, or a datatype (not both).
  */
  SERD_LITERAL = 1,

  /**
     URI (absolute or relative).

     Value is an unquoted URI string, which is either a relative reference
     with respect to the current base URI (e.g. "foo/bar"), or an absolute
     URI (e.g. "http://example.org/foo").
     @see [RFC3986](http://tools.ietf.org/html/rfc3986)
  */
  SERD_URI = 2,

  /**
     CURIE, a shortened URI.

     Value is an unquoted CURIE string relative to the current environment,
     e.g. "rdf:type".  @see [CURIE Syntax 1.0](http://www.w3.org/TR/curie)
  */
  SERD_CURIE = 3,

  /**
     A blank node.

     Value is a blank node ID without any syntactic prefix, like "id3", which
     is meaningful only within this serialisation.  @see [RDF 1.1
     Turtle](http://www.w3.org/TR/turtle/#grammar-production-BLANK_NODE_LABEL)
  */
  SERD_BLANK = 4
} SerdType;

/// Flags indicating certain string properties relevant to serialisation
typedef enum {
  SERD_HAS_NEWLINE = 1u << 0u, ///< Contains line breaks ('\\n' or '\\r')
  SERD_HAS_QUOTE   = 1u << 1u  ///< Contains quotes ('"')
} SerdNodeFlag;

/// Bitwise OR of SerdNodeFlag values
typedef uint32_t SerdNodeFlags;

/// A syntactic RDF node
typedef struct {
  const uint8_t* SERD_NULLABLE buf;     ///< Value string
  size_t                       n_bytes; ///< Size in bytes (excluding null)
  size_t                       n_chars; ///< String length (excluding null)
  SerdNodeFlags                flags;   ///< Node flags (string properties)
  SerdType                     type;    ///< Node type
} SerdNode;

/// An unterminated string fragment
typedef struct {
  const uint8_t* SERD_NULLABLE buf; ///< Start of chunk
  size_t                       len; ///< Length of chunk in bytes
} SerdChunk;

/// An error description
typedef struct {
  SerdStatus                   status;   ///< Error code
  const uint8_t* SERD_NULLABLE filename; ///< File with error
  unsigned                     line;     ///< Line in file with error or 0
  unsigned                     col;      ///< Column in file with error
  const char* SERD_NONNULL     fmt;      ///< Printf-style format string
  va_list* SERD_NONNULL        args;     ///< Arguments for fmt
} SerdError;

/**
   A parsed URI

   This struct directly refers to chunks in other strings, it does not own any
   memory itself.  Thus, URIs can be parsed and/or resolved against a base URI
   in-place without allocating memory.
*/
typedef struct {
  SerdChunk scheme;    ///< Scheme
  SerdChunk authority; ///< Authority
  SerdChunk path_base; ///< Path prefix if relative
  SerdChunk path;      ///< Path suffix
  SerdChunk query;     ///< Query
  SerdChunk fragment;  ///< Fragment
} SerdURI;

/**
   Syntax style options.

   These flags allow more precise control of writer output style.  Note that
   some options are only supported for some syntaxes, for example, NTriples
   does not support abbreviation and is always ASCII.
*/
typedef enum {
  SERD_STYLE_ABBREVIATED = 1u << 0u, ///< Abbreviate triples when possible.
  SERD_STYLE_ASCII       = 1u << 1u, ///< Escape all non-ASCII characters.
  SERD_STYLE_RESOLVED    = 1u << 2u, ///< Resolve URIs against base URI.
  SERD_STYLE_CURIED      = 1u << 3u, ///< Shorten URIs into CURIEs.
  SERD_STYLE_BULK        = 1u << 4u, ///< Write output in pages.
} SerdStyle;

/**
   Free memory allocated by Serd

   This function exists because some systems require memory allocated by a
   library to be freed by code in the same library.  It is otherwise equivalent
   to the standard C free() function.
*/
SERD_API
void
serd_free(void* SERD_NULLABLE ptr);

/**
   @defgroup serd_string String Utilities
   @{
*/

/// Return a string describing a status code
SERD_CONST_API
const uint8_t* SERD_NONNULL
serd_strerror(SerdStatus status);

/**
   Measure a UTF-8 string.

   @return Length of `str` in characters (except NULL).
   @param str A null-terminated UTF-8 string.
   @param n_bytes (Output) Set to the size of `str` in bytes (except NULL).
   @param flags (Output) Set to the applicable flags.
*/
SERD_API
size_t
serd_strlen(const uint8_t* SERD_NONNULL  str,
            size_t* SERD_NULLABLE        n_bytes,
            SerdNodeFlags* SERD_NULLABLE flags);

/**
   Parse a string to a double.

   The API of this function is identical to the standard C strtod function,
   except this function is locale-independent and always matches the lexical
   format used in the Turtle grammar (the decimal point is always ".").
*/
SERD_API
double
serd_strtod(const char* SERD_NONNULL str,
            char* SERD_NONNULL* SERD_NULLABLE endptr);

/**
   Decode a base64 string.

   This function can be used to deserialise a blob node created with
   serd_node_new_blob().

   @param str Base64 string to decode.
   @param len The length of `str`.
   @param size Set to the size of the returned blob in bytes.
   @return A newly allocated blob which must be freed with serd_free().
*/
SERD_API
void* SERD_ALLOCATED
serd_base64_decode(const uint8_t* SERD_NONNULL str,
                   size_t                      len,
                   size_t* SERD_NONNULL        size);

/**
   @}
   @defgroup serd_streams Byte Streams
   @{
*/

/**
   Function to detect I/O stream errors.

   Identical semantics to `ferror`.

   @return Non-zero if `stream` has encountered an error.
*/
typedef int (*SerdStreamErrorFunc)(void* SERD_NONNULL stream);

/**
   Source function for raw string input.

   Identical semantics to `fread`, but may set errno for more informative error
   reporting than supported by SerdStreamErrorFunc.

   @param buf Output buffer.
   @param size Size of a single element of data in bytes (always 1).
   @param nmemb Number of elements to read.
   @param stream Stream to read from (FILE* for fread).
   @return Number of elements (bytes) read.
*/
typedef size_t (*SerdSource)(void* SERD_NONNULL buf,
                             size_t             size,
                             size_t             nmemb,
                             void* SERD_NONNULL stream);

/// Sink function for raw string output
typedef size_t (*SerdSink)(const void* SERD_NONNULL buf,
                           size_t                   len,
                           void* SERD_NONNULL       stream);

/**
   @}
   @defgroup serd_uri URI
   @{
*/

static const SerdURI SERD_URI_NULL =
  {{NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}};

#ifndef SERD_DISABLE_DEPRECATED

/**
   Return the local path for `uri`, or NULL if `uri` is not a file URI.
   Note this (inappropriately named) function only removes the file scheme if
   necessary, and returns `uri` unmodified if it is an absolute path.  Percent
   encoding and other issues are not handled, to properly convert a file URI to
   a path, use serd_file_uri_parse().
*/
SERD_API
SERD_DEPRECATED_BY("serd_file_uri_parse")
const uint8_t* SERD_NULLABLE
serd_uri_to_path(const uint8_t* SERD_NONNULL uri);

#endif

/**
   Get the unescaped path and hostname from a file URI.

   The returned path and `*hostname` must be freed with serd_free().

   @param uri A file URI.
   @param hostname If non-NULL, set to the hostname, if present.
   @return The path component of the URI.
*/
SERD_API
uint8_t* SERD_NULLABLE
serd_file_uri_parse(const uint8_t* SERD_NONNULL uri,
                    uint8_t* SERD_NONNULL* SERD_NULLABLE hostname);

/// Return true iff `utf8` starts with a valid URI scheme
SERD_PURE_API
bool
serd_uri_string_has_scheme(const uint8_t* SERD_NULLABLE utf8);

/// Parse `utf8`, writing result to `out`
SERD_API
SerdStatus
serd_uri_parse(const uint8_t* SERD_NONNULL utf8, SerdURI* SERD_NONNULL out);

/**
   Set target `t` to reference `r` resolved against `base`.

   @see [RFC3986 5.2.2](http://tools.ietf.org/html/rfc3986#section-5.2.2)
*/
SERD_API
void
serd_uri_resolve(const SerdURI* SERD_NONNULL r,
                 const SerdURI* SERD_NONNULL base,
                 SerdURI* SERD_NONNULL       t);

/// Serialise `uri` with a series of calls to `sink`
SERD_API
size_t
serd_uri_serialise(const SerdURI* SERD_NONNULL uri,
                   SerdSink SERD_NONNULL       sink,
                   void* SERD_NONNULL          stream);

/**
   Serialise `uri` relative to `base` with a series of calls to `sink`

   The `uri` is written as a relative URI iff if it a child of `base` and
   `root`.  The optional `root` parameter must be a prefix of `base` and can be
   used keep up-references ("../") within a certain namespace.
*/
SERD_API
size_t
serd_uri_serialise_relative(const SerdURI* SERD_NONNULL  uri,
                            const SerdURI* SERD_NULLABLE base,
                            const SerdURI* SERD_NULLABLE root,
                            SerdSink SERD_NONNULL        sink,
                            void* SERD_NONNULL           stream);

/**
   @}
   @defgroup serd_node Node
   @{
*/

static const SerdNode SERD_NODE_NULL = {NULL, 0, 0, 0, SERD_NOTHING};

/**
   Make a (shallow) node from `str`.

   This measures, but does not copy, `str`.  No memory is allocated.
*/
SERD_API
SerdNode
serd_node_from_string(SerdType type, const uint8_t* SERD_NULLABLE str);

/**
   Make a (shallow) node from a prefix of `str`.

   This measures, but does not copy, `str`.  No memory is allocated.
   Note that the returned node may not be null terminated.
*/
SERD_API
SerdNode
serd_node_from_substring(SerdType                     type,
                         const uint8_t* SERD_NULLABLE str,
                         size_t                       len);

/// Simple wrapper for serd_node_new_uri() to resolve a URI node
SERD_API
SerdNode
serd_node_new_uri_from_node(const SerdNode* SERD_NONNULL uri_node,
                            const SerdURI* SERD_NULLABLE base,
                            SerdURI* SERD_NULLABLE       out);

/// Simple wrapper for serd_node_new_uri() to resolve a URI string
SERD_API
SerdNode
serd_node_new_uri_from_string(const uint8_t* SERD_NULLABLE str,
                              const SerdURI* SERD_NULLABLE base,
                              SerdURI* SERD_NULLABLE       out);

/**
   Create a new file URI node from a file system path and optional hostname.

   Backslashes in Windows paths will be converted and '%' will always be
   percent encoded.  If `escape` is true, all other invalid characters will be
   percent encoded as well.

   If `path` is relative, `hostname` is ignored.
   If `out` is not NULL, it will be set to the parsed URI.
*/
SERD_API
SerdNode
serd_node_new_file_uri(const uint8_t* SERD_NONNULL  path,
                       const uint8_t* SERD_NULLABLE hostname,
                       SerdURI* SERD_NULLABLE       out,
                       bool                         escape);

/**
   Create a new node by serialising `uri` into a new string.

   @param uri The URI to serialise.

   @param base Base URI to resolve `uri` against (or NULL for no resolution).

   @param out Set to the parsing of the new URI (i.e. points only to
   memory owned by the new returned node).
*/
SERD_API
SerdNode
serd_node_new_uri(const SerdURI* SERD_NONNULL  uri,
                  const SerdURI* SERD_NULLABLE base,
                  SerdURI* SERD_NULLABLE       out);

/**
   Create a new node by serialising `uri` into a new relative URI.

   @param uri The URI to serialise.

   @param base Base URI to make `uri` relative to, if possible.

   @param root Root URI for resolution (see serd_uri_serialise_relative()).

   @param out Set to the parsing of the new URI (i.e. points only to
   memory owned by the new returned node).
*/
SERD_API
SerdNode
serd_node_new_relative_uri(const SerdURI* SERD_NONNULL  uri,
                           const SerdURI* SERD_NULLABLE base,
                           const SerdURI* SERD_NULLABLE root,
                           SerdURI* SERD_NULLABLE       out);

/**
   Create a new node by serialising `d` into an xsd:decimal string

   The resulting node will always contain a `.', start with a digit, and end
   with a digit (i.e. will have a leading and/or trailing `0' if necessary).
   It will never be in scientific notation.  A maximum of `frac_digits` digits
   will be written after the decimal point, but trailing zeros will
   automatically be omitted (except one if `d` is a round integer).

   Note that about 16 and 8 fractional digits are required to precisely
   represent a double and float, respectively.

   @param d The value for the new node.
   @param frac_digits The maximum number of digits after the decimal place.
*/
SERD_API
SerdNode
serd_node_new_decimal(double d, unsigned frac_digits);

/// Create a new node by serialising `i` into an xsd:integer string
SERD_API
SerdNode
serd_node_new_integer(int64_t i);

/**
   Create a node by serialising `buf` into an xsd:base64Binary string

   This function can be used to make a serialisable node out of arbitrary
   binary data, which can be decoded using serd_base64_decode().

   @param buf Raw binary input data.
   @param size Size of `buf`.
   @param wrap_lines Wrap lines at 76 characters to conform to RFC 2045.
*/
SERD_API
SerdNode
serd_node_new_blob(const void* SERD_NONNULL buf, size_t size, bool wrap_lines);

/**
   Make a deep copy of `node`.

   @return a node that the caller must free with serd_node_free().
*/
SERD_API
SerdNode
serd_node_copy(const SerdNode* SERD_NULLABLE node);

/// Return true iff `a` is equal to `b`
SERD_PURE_API
bool
serd_node_equals(const SerdNode* SERD_NONNULL a,
                 const SerdNode* SERD_NONNULL b);

/**
   Free any data owned by `node`.

   Note that if `node` is itself dynamically allocated (which is not the case
   for nodes created internally by serd), it will not be freed.
*/
SERD_API
void
serd_node_free(SerdNode* SERD_NULLABLE node);

/**
   @}
   @defgroup serd_event Event Handlers
   @{
*/

/**
   Sink (callback) for errors.

   @param handle Handle for user data.
   @param error Error description.
*/
typedef SerdStatus (*SerdErrorSink)(void* SERD_NULLABLE           handle,
                                    const SerdError* SERD_NONNULL error);

/**
   Sink (callback) for base URI changes

   Called whenever the base URI of the serialisation changes.
*/
typedef SerdStatus (*SerdBaseSink)(void* SERD_NULLABLE          handle,
                                   const SerdNode* SERD_NONNULL uri);

/**
   Sink (callback) for namespace definitions

   Called whenever a prefix is defined in the serialisation.
*/
typedef SerdStatus (*SerdPrefixSink)(void* SERD_NULLABLE          handle,
                                     const SerdNode* SERD_NONNULL name,
                                     const SerdNode* SERD_NONNULL uri);

/**
   Sink (callback) for statements

   Called for every RDF statement in the serialisation.
*/
typedef SerdStatus (*SerdStatementSink)(
  void* SERD_NULLABLE           handle,
  SerdStatementFlags            flags,
  const SerdNode* SERD_NULLABLE graph,
  const SerdNode* SERD_NONNULL  subject,
  const SerdNode* SERD_NONNULL  predicate,
  const SerdNode* SERD_NONNULL  object,
  const SerdNode* SERD_NULLABLE object_datatype,
  const SerdNode* SERD_NULLABLE object_lang);

/**
   Sink (callback) for anonymous node end markers

   This is called to indicate that the anonymous node with the given
   `value` will no longer be referred to by any future statements
   (i.e. the anonymous serialisation of the node is finished).
*/
typedef SerdStatus (*SerdEndSink)(void* SERD_NULLABLE          handle,
                                  const SerdNode* SERD_NONNULL node);

/**
   @}
   @defgroup serd_env Environment
   @{
*/

/// Create a new environment
SERD_API
SerdEnv* SERD_ALLOCATED
serd_env_new(const SerdNode* SERD_NULLABLE base_uri);

/// Free `env`
SERD_API
void
serd_env_free(SerdEnv* SERD_NULLABLE env);

/// Get the current base URI
SERD_API
const SerdNode* SERD_NONNULL
serd_env_get_base_uri(const SerdEnv* SERD_NONNULL env,
                      SerdURI* SERD_NULLABLE      out);

/// Set the current base URI
SERD_API
SerdStatus
serd_env_set_base_uri(SerdEnv* SERD_NONNULL         env,
                      const SerdNode* SERD_NULLABLE uri);

/**
   Set a namespace prefix

   A namespace prefix is used to expand CURIE nodes, for example, with the
   prefix "xsd" set to "http://www.w3.org/2001/XMLSchema#", "xsd:decimal" will
   expand to "http://www.w3.org/2001/XMLSchema#decimal".
*/
SERD_API
SerdStatus
serd_env_set_prefix(SerdEnv* SERD_NONNULL        env,
                    const SerdNode* SERD_NONNULL name,
                    const SerdNode* SERD_NONNULL uri);

/// Set a namespace prefix
SERD_API
SerdStatus
serd_env_set_prefix_from_strings(SerdEnv* SERD_NONNULL       env,
                                 const uint8_t* SERD_NONNULL name,
                                 const uint8_t* SERD_NONNULL uri);

/// Qualify `uri` into a CURIE if possible
SERD_API
bool
serd_env_qualify(const SerdEnv* SERD_NONNULL  env,
                 const SerdNode* SERD_NONNULL uri,
                 SerdNode* SERD_NONNULL       prefix,
                 SerdChunk* SERD_NONNULL      suffix);

/**
   Expand `curie`.

   Errors: SERD_ERR_BAD_ARG if `curie` is not valid, or SERD_ERR_BAD_CURIE if
   prefix is not defined in `env`.
*/
SERD_API
SerdStatus
serd_env_expand(const SerdEnv* SERD_NONNULL  env,
                const SerdNode* SERD_NONNULL curie,
                SerdChunk* SERD_NONNULL      uri_prefix,
                SerdChunk* SERD_NONNULL      uri_suffix);

/**
   Expand `node`, which must be a CURIE or URI, to a full URI.

   Returns null if `node` can not be expanded.
*/
SERD_API
SerdNode
serd_env_expand_node(const SerdEnv* SERD_NONNULL  env,
                     const SerdNode* SERD_NONNULL node);

/// Call `func` for each prefix defined in `env`
SERD_API
void
serd_env_foreach(const SerdEnv* SERD_NONNULL env,
                 SerdPrefixSink SERD_NONNULL func,
                 void* SERD_NULLABLE         handle);

/**
   @}
   @defgroup serd_reader Reader
   @{
*/

/// Create a new RDF reader
SERD_API
SerdReader* SERD_ALLOCATED
serd_reader_new(SerdSyntax          syntax,
                void* SERD_NULLABLE handle,
                void (*SERD_NULLABLE free_handle)(void* SERD_NULLABLE),
                SerdBaseSink SERD_NULLABLE      base_sink,
                SerdPrefixSink SERD_NULLABLE    prefix_sink,
                SerdStatementSink SERD_NULLABLE statement_sink,
                SerdEndSink SERD_NULLABLE       end_sink);

/**
   Enable or disable strict parsing

   The reader is non-strict (lax) by default, which will tolerate URIs with
   invalid characters.  Setting strict will fail when parsing such files.  An
   error is printed for invalid input in either case.
*/
SERD_API
void
serd_reader_set_strict(SerdReader* SERD_NONNULL reader, bool strict);

/**
   Set a function to be called when errors occur during reading.

   The `error_sink` will be called with `handle` as its first argument.  If
   no error function is set, errors are printed to stderr in GCC style.
*/
SERD_API
void
serd_reader_set_error_sink(SerdReader* SERD_NONNULL    reader,
                           SerdErrorSink SERD_NULLABLE error_sink,
                           void* SERD_NULLABLE         error_handle);

/// Return the `handle` passed to serd_reader_new()
SERD_PURE_API
void* SERD_NULLABLE
serd_reader_get_handle(const SerdReader* SERD_NONNULL reader);

/**
   Set a prefix to be added to all blank node identifiers.

   This is useful when multiple files are to be parsed into the same output (a
   model or a file).  Since Serd preserves blank node IDs, this could cause
   conflicts where two non-equivalent blank nodes are merged, resulting in
   corrupt data.  By setting a unique blank node prefix for each parsed file,
   this can be avoided, while preserving blank node names.
*/
SERD_API
void
serd_reader_add_blank_prefix(SerdReader* SERD_NONNULL     reader,
                             const uint8_t* SERD_NULLABLE prefix);

/**
   Set the URI of the default graph.

   If this is set, the reader will emit quads with the graph set to the given
   node for any statements that are not in a named graph (which is currently
   all of them since Serd currently does not support any graph syntaxes).
*/
SERD_API
void
serd_reader_set_default_graph(SerdReader* SERD_NONNULL      reader,
                              const SerdNode* SERD_NULLABLE graph);

/// Read a file at a given `uri`
SERD_API
SerdStatus
serd_reader_read_file(SerdReader* SERD_NONNULL    reader,
                      const uint8_t* SERD_NONNULL uri);

/**
   Start an incremental read from a file handle.

   Iff `bulk` is true, `file` will be read a page at a time.  This is more
   efficient, but uses a page of memory and means that an entire page of input
   must be ready before any callbacks will fire.  To react as soon as input
   arrives, set `bulk` to false.
*/
SERD_API
SerdStatus
serd_reader_start_stream(SerdReader* SERD_NONNULL     reader,
                         FILE* SERD_NONNULL           file,
                         const uint8_t* SERD_NULLABLE name,
                         bool                         bulk);

/**
   Start an incremental read from a user-specified source.

   The `read_func` is guaranteed to only be called for `page_size` elements
   with size 1 (i.e. `page_size` bytes).
*/
SERD_API
SerdStatus
serd_reader_start_source_stream(SerdReader* SERD_NONNULL         reader,
                                SerdSource SERD_NONNULL          read_func,
                                SerdStreamErrorFunc SERD_NONNULL error_func,
                                void* SERD_NONNULL               stream,
                                const uint8_t* SERD_NULLABLE     name,
                                size_t                           page_size);

/**
   Read a single "chunk" of data during an incremental read

   This function will read a single top level description, and return.  This
   may be a directive, statement, or several statements; essentially it reads
   until a '.' is encountered.  This is particularly useful for reading
   directly from a pipe or socket.
*/
SERD_API
SerdStatus
serd_reader_read_chunk(SerdReader* SERD_NONNULL reader);

/// Finish an incremental read from a file handle
SERD_API
SerdStatus
serd_reader_end_stream(SerdReader* SERD_NONNULL reader);

/// Read `file`
SERD_API
SerdStatus
serd_reader_read_file_handle(SerdReader* SERD_NONNULL     reader,
                             FILE* SERD_NONNULL           file,
                             const uint8_t* SERD_NULLABLE name);

/// Read a user-specified byte source
SERD_API
SerdStatus
serd_reader_read_source(SerdReader* SERD_NONNULL         reader,
                        SerdSource SERD_NONNULL          source,
                        SerdStreamErrorFunc SERD_NONNULL error,
                        void* SERD_NONNULL               stream,
                        const uint8_t* SERD_NULLABLE     name,
                        size_t                           page_size);

/// Read `utf8`
SERD_API
SerdStatus
serd_reader_read_string(SerdReader* SERD_NONNULL    reader,
                        const uint8_t* SERD_NONNULL utf8);

/// Free `reader`
SERD_API
void
serd_reader_free(SerdReader* SERD_NULLABLE reader);

/**
   @}
   @defgroup serd_writer Writer
   @{
*/

/// Create a new RDF writer
SERD_API
SerdWriter* SERD_ALLOCATED
serd_writer_new(SerdSyntax                   syntax,
                SerdStyle                    style,
                SerdEnv* SERD_NONNULL        env,
                const SerdURI* SERD_NULLABLE base_uri,
                SerdSink SERD_NONNULL        ssink,
                void* SERD_NULLABLE          stream);

/// Free `writer`
SERD_API
void
serd_writer_free(SerdWriter* SERD_NULLABLE writer);

/// Return the env used by `writer`
SERD_PURE_API
SerdEnv* SERD_NONNULL
serd_writer_get_env(SerdWriter* SERD_NONNULL writer);

/**
   A convenience sink function for writing to a FILE*.

   This function can be used as a SerdSink when writing to a FILE*.  The
   `stream` parameter must be a FILE* opened for writing.
*/
SERD_API
size_t
serd_file_sink(const void* SERD_NONNULL buf,
               size_t                   len,
               void* SERD_NONNULL       stream);

/**
   A convenience sink function for writing to a string

   This function can be used as a SerdSink to write to a SerdChunk which is
   resized as necessary with realloc().  The `stream` parameter must point to
   an initialized SerdChunk.  When the write is finished, the string should be
   retrieved with serd_chunk_sink_finish().
*/
SERD_API
size_t
serd_chunk_sink(const void* SERD_NONNULL buf,
                size_t                   len,
                void* SERD_NONNULL       stream);

/**
   Finish a serialisation to a chunk with serd_chunk_sink().

   The returned string is the result of the serialisation, which is null
   terminated (by this function) and owned by the caller.
*/
SERD_API
uint8_t* SERD_NULLABLE
serd_chunk_sink_finish(SerdChunk* SERD_NONNULL stream);

/**
   Set a function to be called when errors occur during writing.

   The `error_sink` will be called with `handle` as its first argument.  If
   no error function is set, errors are printed to stderr.
*/
SERD_API
void
serd_writer_set_error_sink(SerdWriter* SERD_NONNULL   writer,
                           SerdErrorSink SERD_NONNULL error_sink,
                           void* SERD_NULLABLE        error_handle);

/**
   Set a prefix to be removed from matching blank node identifiers

   This is the counterpart to serd_reader_add_blank_prefix() which can be used
   to "undo" added prefixes.
*/
SERD_API
void
serd_writer_chop_blank_prefix(SerdWriter* SERD_NONNULL     writer,
                              const uint8_t* SERD_NULLABLE prefix);

/**
   Set the current output base URI, and emit a directive if applicable.

   Note this function can be safely casted to SerdBaseSink.
*/
SERD_API
SerdStatus
serd_writer_set_base_uri(SerdWriter* SERD_NONNULL      writer,
                         const SerdNode* SERD_NULLABLE uri);

/**
   Set the current root URI.

   The root URI should be a prefix of the base URI.  The path of the root URI
   is the highest path any relative up-reference can refer to.  For example,
   with root <file:///foo/root> and base <file:///foo/root/base>,
   <file:///foo/root> will be written as <../>, but <file:///foo> will be
   written non-relatively as <file:///foo>.  If the root is not explicitly set,
   it defaults to the base URI, so no up-references will be created at all.
*/
SERD_API
SerdStatus
serd_writer_set_root_uri(SerdWriter* SERD_NONNULL      writer,
                         const SerdNode* SERD_NULLABLE uri);

/**
   Set a namespace prefix (and emit directive if applicable).

   Note this function can be safely casted to SerdPrefixSink.
*/
SERD_API
SerdStatus
serd_writer_set_prefix(SerdWriter* SERD_NONNULL     writer,
                       const SerdNode* SERD_NONNULL name,
                       const SerdNode* SERD_NONNULL uri);

/**
   Write a statement.

   Note this function can be safely casted to SerdStatementSink.
*/
SERD_API
SerdStatus
serd_writer_write_statement(SerdWriter* SERD_NONNULL      writer,
                            SerdStatementFlags            flags,
                            const SerdNode* SERD_NULLABLE graph,
                            const SerdNode* SERD_NONNULL  subject,
                            const SerdNode* SERD_NONNULL  predicate,
                            const SerdNode* SERD_NONNULL  object,
                            const SerdNode* SERD_NULLABLE datatype,
                            const SerdNode* SERD_NULLABLE lang);

/**
   Mark the end of an anonymous node's description.

   Note this function can be safely casted to SerdEndSink.
*/
SERD_API
SerdStatus
serd_writer_end_anon(SerdWriter* SERD_NONNULL      writer,
                     const SerdNode* SERD_NULLABLE node);

/**
   Finish a write

   This flushes any pending output, for example terminating punctuation, so
   that the output is a complete document.
*/
SERD_API
SerdStatus
serd_writer_finish(SerdWriter* SERD_NONNULL writer);

/**
   @}
   @}
*/

#ifdef __cplusplus
#  if defined(__GNUC__)
#    pragma GCC diagnostic pop
#  endif
} /* extern "C" */
#endif

#endif /* SERD_SERD_H */
