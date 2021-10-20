import argparse
import os

BUNDLE_TEMPLATE = """juce::lv2::Bundle
{{
"{}",
{{
{}
}}
}}
"""

BUNDLE_RESOURCE_TEMPLATE = """juce::lv2::BundleResource
{{
"{}",
{}
}}
"""

FUNCTION_TEMPLATE = """#pragma once

#include <vector>

namespace juce
{{
namespace lv2
{{

struct BundleResource
{{
    const char* name;
    const char* contents;
}};

struct Bundle
{{
    const char* name;
    std::vector<BundleResource> contents;

    static std::vector<Bundle> getAllBundles();
}};

}}
}}

std::vector<juce::lv2::Bundle> juce::lv2::Bundle::getAllBundles()
{{
    return {{
{}
}};
}}
"""


def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]


def get_chunked_string_literal(s):
    return ' '.join(map(lambda x: 'R"lv2ttl({})lv2ttl"'.format(''.join(x)), chunks(s, 8000)))


def get_file_source_string(ttl):
    with open(ttl) as f:
        return BUNDLE_RESOURCE_TEMPLATE.format(os.path.basename(ttl),
                                               get_chunked_string_literal(f.read()))


def generate_bundle_source(root, files):
    if len(files) == 0:
        return ""

    return BUNDLE_TEMPLATE.format(os.path.basename(root),
                                  ", ".join(get_file_source_string(os.path.join(root, ttl)) for ttl in files))

def filter_turtle(files):
    return [f for f in files if f.endswith(".ttl")]


def filter_ttl_files(lv2_dir):
    for root, _, files in os.walk(args.lv2_dir):
        yield root, filter_turtle(files)


parser = argparse.ArgumentParser()
parser.add_argument("lv2_dir")
args = parser.parse_args()

print(FUNCTION_TEMPLATE.format(", ".join(generate_bundle_source(root, files)
                                         for root, files in filter_ttl_files(args.lv2_dir)
                                         if len(files) != 0)),
      end = "\r\n")
