#!/usr/bin/env python3
"""Vendors libwebp into the directory containing this script.

To update the library, bump VERSION -- a release number, vendored from its
v-prefixed tag, or a full commit hash -- and run this script. It replaces the
contents of this directory (everything except the script itself) with a fresh
copy of the upstream sources, reduced to the files the JUCE unity translation
units need (the .c files upstream compiles into libwebp and libsharpyuv on x86
and Arm, and every header reachable from them), rewrites the sources' includes
for the JUCE include model (only the modules directory is on the include path),
applies the JUCE-specific fixes in PATCHES, and writes a JUCE_UPSTREAM.txt.
Needs git and Python 3.10+.
"""

import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

REPO = "https://chromium.googlesource.com/webm/libwebp"
VERSION = "1.6.0"

# The files the closure starts from, besides the sources below: the licence and
# the notices it refers to.
ROOTS = ["AUTHORS", "COPYING", "PATENTS"]

# The variables in upstream's automake files that list the sources of libwebp
# and libsharpyuv, minus the MIPS and MSA variants. Animation support (mux,
# demux) is not vendored.
SOURCE_LISTS = {
    "sharpyuv/Makefile.am": ["libsharpyuv_la_SOURCES", "libsharpyuv_sse2_la_SOURCES",
                             "libsharpyuv_neon_la_SOURCES"],
    "src/dec/Makefile.am": ["libwebpdecode_la_SOURCES"],
    "src/dsp/Makefile.am": ["COMMON_SOURCES", "ENC_SOURCES",
                            "libwebpdspdecode_sse2_la_SOURCES", "libwebpdsp_sse2_la_SOURCES",
                            "libwebpdspdecode_sse41_la_SOURCES", "libwebpdsp_sse41_la_SOURCES",
                            "libwebpdspdecode_avx2_la_SOURCES", "libwebpdsp_avx2_la_SOURCES",
                            "libwebpdspdecode_neon_la_SOURCES", "libwebpdsp_neon_la_SOURCES"],
    "src/enc/Makefile.am": ["libwebpencode_la_SOURCES"],
    "src/utils/Makefile.am": ["COMMON_SOURCES", "ENC_SOURCES"],
}

# Upstream's include search directories: the repository root, and src for
# sharpyuv.
SEARCH_DIRS = [".", "src"]

INCLUDE_RE = re.compile(r'^(\s*#\s*include\s*)(["<])([^">]+)([">])', re.M)

GCC_AVX2_SHIMS = r"""// Shims for AVX2 intrinsics missing from older GCC. Clang is excluded (it
// defines __GNUC__ as 4 but has always provided these). The version gates
// must stay exact: once GCC provides the real intrinsic, a shim with the
// same name fails to compile.
#if defined(WEBP_USE_AVX2) && defined(__GNUC__) && !defined(__clang__) && \
    (__GNUC__ < 11)
#include <immintrin.h>

// Added in GCC 11 (GCC PR target/95483).
static WEBP_INLINE int _mm256_cvtsi256_si32(__m256i a) {
    return _mm_cvtsi128_si32(_mm256_castsi256_si128(a));
}

#if (__GNUC__ < 10)
// Added in GCC 10 (GCC PR target/91341).
static WEBP_INLINE void _mm256_storeu2_m128i(__m128i* hi, __m128i* lo,
                                              __m256i a) {
    _mm_storeu_si128(lo, _mm256_castsi256_si128(a));
    _mm_storeu_si128(hi, _mm256_extracti128_si256(a, 1));
}
#endif  // __GNUC__ < 10

#endif  // WEBP_USE_AVX2 && defined(__GNUC__) && !defined(__clang__) &&
        // __GNUC__ < 11

"""

# JUCE-specific fixes as (file, expected number of occurrences, old, new) exact
# text replacements, so that a change in the upstream text fails loudly instead
# of silently dropping a fix.
PATCHES = [
    # Without HAVE_CONFIG_H, upstream selects the x86 SIMD paths from _M_X64 /
    # _M_IX86, which MSVC also defines for ARM64EC. Upstream's own build system
    # papers over this; we don't use it.
    ("src/dsp/cpu.h", 3,
     "(defined(_M_X64) || defined(_M_IX86))\n",
     "(defined(_M_X64) || defined(_M_IX86)) && !defined(_M_ARM64EC)\n"),
    # Upstream doesn't support GCC 7; two shims are all it takes.
    ("src/dsp/cpu.h", 1,
     "#undef WEBP_MSC_AVX2\n",
     GCC_AVX2_SHIMS + "#undef WEBP_MSC_AVX2\n"),
]

UPSTREAM_NOTE = """Upstream: {repo}
Version:  {version} ({ref})

Vendored by vendor.py in this directory; rerun it to update. It copies the
files upstream compiles into libwebp and libsharpyuv on x86 and Arm and every
header reachable from them, rewrites their includes as paths relative to the
JUCE modules directory, and applies two fixes to src/dsp/cpu.h: one keeping
the x86 SIMD paths off ARM64EC targets, and one shimming the AVX2 intrinsics
missing from GCC before version 11.
"""


# Interprets the provided path constituents relative to the location of this
# script, and returns an absolute Path to the resulting location.
#
# E.g. rel_to_py(".") returns an absolute path to the directory containing this
# script.
def rel_to_py(*paths) -> Path:
    return Path(
        os.path.realpath(
            os.path.join(os.path.realpath(os.path.dirname(__file__)), *paths)
        )
    )


def git(*args) -> str:
    return subprocess.run(
        ["git", *args], check=True, capture_output=True, text=True
    ).stdout.strip()


def is_commit(version: str) -> bool:
    return re.fullmatch(r"[0-9a-f]{40}", version) is not None


def clone(repo: str, version: str, dest: Path):
    if is_commit(version):
        git("clone", "--quiet", repo, str(dest))
        git("-C", str(dest), "checkout", "--quiet", version)
    else:
        git("clone", "--quiet", "--depth", "1", "--branch", f"v{version}", repo, str(dest))


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def write(path: Path, text: str):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8", newline="\n")


def sources_from_am(tree: Path) -> list:
    sources = []
    for am, variables in SOURCE_LISTS.items():
        text = read(tree / am)
        for variable in variables:
            for match in re.finditer(rf"^\s*{variable}\s*\+?=(.*)$", text, re.M):
                sources += [Path(am).parent / s for s in match.group(1).split()]
    return sources


def resolve(tree: Path, file: Path, name: str):
    candidates = [file.parent / name] + [tree / d / name for d in SEARCH_DIRS]
    return next((c.resolve() for c in candidates if c.is_file()), None)


def closure(tree: Path, roots: list) -> set:
    seen, todo = set(), [tree / r for r in roots]
    while todo:
        file = todo.pop()
        if file in seen:
            continue
        assert file.is_file(), file
        seen.add(file)
        if file.suffix not in (".c", ".h"):
            continue
        for match in INCLUDE_RE.finditer(file.read_text(encoding="utf-8", errors="replace")):
            target = resolve(tree, file, match.group(3))
            if target is not None and target.is_relative_to(tree):
                todo.append(target)
    return seen


# JUCE puts the modules directory on the include path, so includes of files in
# the vendored tree become paths from there. Includes that already resolve
# relative to the including file are left alone.
def rewrite_includes(tree: Path, file: Path, prefix: str):
    def module_rooted(match):
        name = match.group(3)
        if (file.parent / name).is_file():
            return match.group(0)
        target = resolve(tree, file, name)
        if target is None:
            return match.group(0)
        return f'{match.group(1)}"{prefix}/{target.relative_to(tree).as_posix()}"'

    write(file, INCLUDE_RE.sub(module_rooted, read(file)))


def patch(tree: Path):
    for name, count, old, new in PATCHES:
        text = read(tree / name)
        assert text.count(old) == count, (name, old)
        write(tree / name, text.replace(old, new))


def main():
    dest = rel_to_py(".")
    prefix = dest.relative_to(dest.parents[2]).as_posix()

    with tempfile.TemporaryDirectory() as tmp:
        tree = Path(tmp).resolve() / "libwebp"
        clone(REPO, VERSION, tree)

        for entry in dest.iterdir():
            if entry.name != Path(__file__).name:
                shutil.rmtree(entry) if entry.is_dir() else entry.unlink()

        for file in closure(tree, ROOTS + sources_from_am(tree)):
            write(dest / file.relative_to(tree), read(file))

        described = git("-C", str(tree), "describe", "--tags", "--match", "v*").removeprefix("v")
        commit = git("-C", str(tree), "rev-parse", "HEAD")
        ref = f"commit {commit}" if is_commit(VERSION) else f"tag v{VERSION}, commit {commit}"
        write(dest / "JUCE_UPSTREAM.txt", UPSTREAM_NOTE.format(repo=REPO, version=described, ref=ref))

    for file in list(dest.rglob("*.c")) + list(dest.rglob("*.h")):
        rewrite_includes(dest, file, prefix)

    patch(dest)


if __name__ == "__main__":
    main()
