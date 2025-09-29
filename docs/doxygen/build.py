from pathlib import Path
import argparse
import os
import subprocess
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element
import re

def get_module_description(module_header: Path) -> str:
    with open(module_header, 'r') as f:
        content = f.read()
        declaration = content[content.find('BEGIN_JUCE_MODULE_DECLARATION'):content.find('END_JUCE_MODULE_DECLARATION')]
        for line in map(lambda x: x.strip(), declaration.split("\n")):
            if line.startswith('description:'):
                return line.removeprefix('description:').strip()

def createContentDict() -> dict:
    return {
        'classes/structs': {},
        'functions': {},
        'typedefs': {},
        'macros': {},
        'enums': {},
        'dirs': {}
    }

def getContentObjectFromPath(root_object: dict, location: str) -> dict:
    path_components = Path(location).parts
    contents = root_object[path_components[0]]
    for path_component in path_components[1:-1]:
        if not (path_component in contents['dirs']):
            contents['dirs'][path_component] = createContentDict()
        contents = contents['dirs'][path_component]
    return contents

def parseNamespaceXml(root_object, filename):
    element = ET.parse(f'xml/{filename}').getroot()

    for class_reference in element.findall('.//innerclass'):
        refid = class_reference.attrib['refid']
        html_filename = f'{refid}.html'
        if not Path(f'doc/{html_filename}').is_file():
            continue
        xml_path = Path(f'xml/{refid}.xml')
        if not xml_path.is_file():
            continue
        class_element = ET.parse(xml_path).getroot()
        location = class_element.find('.//location').attrib['file']
        contents = getContentObjectFromPath(root_object, location)
        contents['classes/structs'][class_reference.text.removeprefix('juce::')] = {
            'url': f'./{html_filename}'
        }
    
    namespace_def_id = element.find('.//compounddef').attrib['id']
    namespace_id_prefix = f'{namespace_def_id}_1'
    
    for kind, contents_key in (('function', 'functions'), ('typedef', 'typedefs'), ('enum', 'enums')):
        for reference in element.findall(f".//memberdef[@kind='{kind}']"):
            desc = reference.find('briefdescription')
            if (len(desc) == 0) and (not desc.text.strip()):
                continue
            ref_id = reference.attrib['id']
            if not ref_id.startswith(namespace_id_prefix):
                continue
            name = reference.find('.//qualifiedname').text.removeprefix('juce::')
            if re.search(r'operator[+\-*/=!<>]*$', name):
                continue
            anchor = ref_id.removeprefix(namespace_id_prefix)
            location = reference.find('.//location').attrib['file']
            contents = getContentObjectFromPath(modules, location)
            contents[contents_key][name] = {
                'url': f'./{namespace_def_id}.html#{anchor}'
            }

def parseFileXml(root_object, filename):
    element = ET.parse(f'xml/{filename}').getroot() 
    compounddef_id = element.find('compounddef').attrib['id']
    def_reference_link_prefix = compounddef_id + '_1'
    for def_reference in element.findall(".//memberdef[@kind='define']"):
        def_id = def_reference.attrib['id']
        if not def_id.startswith(def_reference_link_prefix):
            continue
        def_desc = def_reference.find('briefdescription')
        if (len(def_desc) == 0) and (not def_desc.text.strip()):
            continue
        html_filename = f'{compounddef_id}.html'
        if not (Path('doc') / html_filename).exists():
            continue
        location = def_reference.find('.//location').attrib['file']
        contents = getContentObjectFromPath(root_object, location) 
        anchor = def_id.removeprefix(def_reference_link_prefix)
        contents['macros'][def_reference.find('name').text] = {
            'url': f'./{html_filename}#{anchor}',
        }

def write_module_html_recursive(parent: Element, data: dict):
    for section_key, section_contents in data.items():
        if section_key in ('dirs', 'description'):
            continue
        if not section_contents:
            continue
        #ET.SubElement(parent, 'span', {'class': 'juce-module-contents-title'}).text = section_key
        section_list = ET.SubElement(parent, 'ul', {'class': 'juce-module-class-list'})
        for section_item_key, section_item_contents in sorted(section_contents.items()):
            item = ET.SubElement(section_list, 'li', {'class': 'juce-module-class-item'})
            ET.SubElement(item, 'a', {'href': section_item_contents['url'], 'class': 'juce-module-class-link'}).text = section_item_key
    if data['dirs']:
        dir_table = ET.SubElement(parent, 'table', {'class': 'juce-module-dir-table'})
        for dir_key, dir_contents in sorted(data['dirs'].items()):
            row = ET.SubElement(dir_table, 'tr', {'class': 'juce-module-dir-row'})
            ET.SubElement(row, 'td', {'class': 'juce-module-dir-name'}).text = dir_key
            content = ET.SubElement(row, 'td', {'class': 'juce-module-dir-contents'})
            write_module_html_recursive(content, dir_contents)

parser = argparse.ArgumentParser()
parser.add_argument('--sitemap-url', help='The Doxygen sitemap configuration variable')
args = parser.parse_args()

if args.sitemap_url:
    os.environ['JUCE_SITEMAP_URL']=args.sitemap_url

print('--- Running Doxygen')
subprocess.run("doxygen", shell=True, check=True)

print('--- Parsing module headers')
modules_dir = Path('../../modules')
assert modules_dir.is_dir()
modules = {}
for module_name in os.listdir(modules_dir):
    module_dir = modules_dir / module_name
    if not module_dir.is_dir():
        continue
    module_header = module_dir / f'{module_name}.h'
    if not module_header.is_file():
        continue
    modules[module_name] = createContentDict()
    modules[module_name]['description'] = get_module_description(module_header)

print('--- Parsing Doxygen XML')
for xml_filename in os.listdir('xml'):
    if xml_filename.startswith('namespacejuce'):
        parseNamespaceXml(modules, xml_filename)
    elif xml_filename.startswith('juce__'):
        # There are no macros in the namespace XML so we need to get them separately
        parseFileXml(modules, xml_filename)

print('--- Creating JUCE Module HTML')

module_icon = ET.Element('svg', {'fill': '#000000', 'viewBox': '0 0 56 56', 'class': 'juce-module-icon'})
ET.SubElement(module_icon, 'path', {'d': 'M 28.0000 26.6406 L 50.0783 14.1016 C 49.7264 13.75 49.3045 13.4688 48.7890 13.1875 L 32.2657 3.7657 C 30.8126 2.9453 29.4063 2.5000 28.0000 2.5000 C 26.5938 2.5000 25.1875 2.9453 23.7344 3.7657 L 7.2110 13.1875 C 6.6954 13.4688 6.2735 13.75 5.9219 14.1016 Z M 26.4063 53.5 L 26.4063 29.4532 L 4.3985 16.8906 C 4.2813 17.4063 4.2110 17.9688 4.2110 18.6719 L 4.2110 36.9297 C 4.2110 40.3281 5.4063 41.5938 7.5860 42.8360 L 25.9375 53.2891 C 26.1016 53.3828 26.2422 53.4532 26.4063 53.5 Z M 29.5938 53.5 C 29.7579 53.4532 29.8985 53.3828 30.0626 53.2891 L 48.4141 42.8360 C 50.5938 41.5938 51.7890 40.3281 51.7890 36.9297 L 51.7890 18.6719 C 51.7890 17.9688 51.7189 17.4063 51.6018 16.8906 L 29.5938 29.4532 Z'})

main_div = ET.Element('div', {'class': 'juce-modules-continer'})

toc_div = ET.SubElement(main_div, 'div', {'class': 'juce-module-toc-container'})
ET.SubElement(toc_div, 'p', {'class': 'juce-module-toc-desc'}).text = "Here is a summary of the JUCE modules. To search absolutely everything please use the search bar."
toc_table = ET.SubElement(toc_div, 'table', {'class': 'juce-module-toc-table'})
for key, contents in sorted(modules.items()):
    toc_row = ET.SubElement(toc_table, 'tr', {'class': 'juce-module-toc-row'})
    module_toc_name = ET.SubElement(toc_row, 'td', {'class': 'juce-module-toc-module-name'})
    ET.SubElement(module_toc_name, 'a', {'href': f'#{key}', 'class': 'juce-module-toc-module-name-link'}).text = key
    ET.SubElement(toc_row, 'td', {'class': 'juce-module-toc-module-decs'}).text = contents['description']
ET.SubElement(main_div, 'div', {'class': 'juce-module-toc-divider'})

for key, contents in sorted(modules.items()):
    module_div = ET.SubElement(main_div, 'div', {'class': 'juce-module'})
    module_header_div = ET.SubElement(module_div, 'div', {'class': 'juce-module-header'}) 
    module_title_div = ET.SubElement(module_header_div, 'div', {'class': 'juce-module-title'}) 
    module_title_div.append(module_icon)
    ET.SubElement(module_title_div, 'span', {'id': key, 'class': 'juce-module-name'}).text = key
    ET.SubElement(module_header_div, 'span', {'class': 'juce-module-desc'}).text = contents['description']
    module_contents_div = ET.SubElement(module_div, 'div', {'class': 'juce-module-contents'}) 
    write_module_html_recursive(module_contents_div, contents)

print('--- Updating Doxygen HTML')
html = ET.tostring(main_div, encoding='utf-8', method='html').decode('utf-8')
with open('doc/index.html', 'r', encoding='utf-8') as f:
    content = f.read()
content = content.replace('JUCE Documentation', 'JUCE Modules')
key = '<div class="contents">'
index = content.index(key) + len(key)
content = content[:index] + html + content[index:]
with open('doc/index.html', 'w', encoding='utf-8') as f:
    f.write(content)
