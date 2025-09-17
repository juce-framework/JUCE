import argparse
import os
import subprocess
from pathlib import Path, PurePath
import xml.etree.ElementTree as ET
from xml.etree.ElementTree import Element

def get_modules_directory_xml_element(xml_dir: Path) -> Element:
    for entry in os.listdir('xml'):
        if not entry.startswith('dir_'):
            continue
        root = ET.parse(xml_dir / entry).getroot()
        location = root.find('.//location')
        path = PurePath(location.attrib['file'])
        referenced_dir_path_parts = path.parts
        if referenced_dir_path_parts[-1] == 'modules':
            return root, path

def get_module_description(module_header: Path) -> str:
    with open(module_header, 'r') as f:
        content = f.read()
        declaration = content[content.find('BEGIN_JUCE_MODULE_DECLARATION'):content.find('END_JUCE_MODULE_DECLARATION')]
        for line in map(lambda x: x.strip(), declaration.split("\n")):
            if line.startswith('description:'):
                return line.removeprefix('description:').strip()

def get_doxygen_items_recursive(xml_file_ref: str, items: list):
    element = ET.parse(f'{Path('xml') / xml_file_ref}.xml').getroot()
    item_type = element.find('compounddef').attrib['kind']
    if item_type == 'dir':
        name = element.find('.//compoundname').text
        new_item = {
            'name': name,
            'classes': [],
            'dirs': []
        }
        items.append(new_item)
        items = new_item['classes']
    for class_reference in element.findall('.//innerclass'):
        html_filename = f'{class_reference.attrib['refid']}.html'
        if (Path('doc') / html_filename).exists():
            items.append({
                'name': class_reference.text.removeprefix('juce::'),
                'url': f'./{html_filename}',
            })
    for file_reference in element.findall('.//innerfile'):
        get_doxygen_items_recursive(file_reference.attrib['refid'], items)
    for dir_reference in element.findall('.//innerdir'):
        get_doxygen_items_recursive(dir_reference.attrib['refid'], new_item['dirs'])

def remove_empty_doxygen_items_recursive(parent: dict):
    parent['dirs'] = list(filter(lambda x: x['classes'] or x['dirs'], parent['dirs']))
    for d in parent['dirs']:
        remove_empty_doxygen_items_recursive(d)

def write_module_html_recursive(parent: Element, data: dict):
    if data['classes']:
        class_list = ET.SubElement(parent, 'ul', {'class': 'juce-module-class-list'})
        for c in data['classes']:
            item = ET.SubElement(class_list, 'li', {'class': 'juce-module-class-item'})
            ET.SubElement(item, 'a', {'href': c['url'], 'class': 'juce-module-class-link'}).text = c['name']
    if data['dirs']:
        dir_table = ET.SubElement(parent, 'table', {'class': 'juce-module-dir-table'})
        for d in data['dirs']:
            row = ET.SubElement(dir_table, 'tr', {'class': 'juce-module-dir-row'})
            ET.SubElement(row, 'td', {'class': 'juce-module-dir-name'}).text = d['name']
            content = ET.SubElement(row, 'td', {'class': 'juce-module-dir-contents'})
            write_module_html_recursive(content, d)
    

parser = argparse.ArgumentParser()
parser.add_argument('--sitemap-url', help='a sitemap URL for Doxygen')
args = parser.parse_args()

if args.sitemap_url:
    os.environ['JUCE_SITEMAP_URL']=args.sitemap_url

print('--- Running Doxygen')
subprocess.run("doxygen", shell=True, check=True)

print('--- Parsing Doxygen XML')
xml_dir = Path('xml')
root, root_path = get_modules_directory_xml_element(xml_dir)
modules = []
module_descriptions = {}
for dir_reference in root.findall('.//innerdir'):
    module_name = dir_reference.text
    module_path = root_path / module_name
    module_header_path = module_path / f'{module_name}.h'
    module_descriptions[module_name] = get_module_description(module_header_path)
    get_doxygen_items_recursive(dir_reference.attrib['refid'], modules)
    remove_empty_doxygen_items_recursive(modules[-1])

print('--- Creating JUCE Module HTML')

module_icon = ET.Element('svg', {'fill': '#000000', 'viewBox': '0 0 56 56', 'class': 'juce-module-icon'})
ET.SubElement(module_icon, 'path', {'d': 'M 28.0000 26.6406 L 50.0783 14.1016 C 49.7264 13.75 49.3045 13.4688 48.7890 13.1875 L 32.2657 3.7657 C 30.8126 2.9453 29.4063 2.5000 28.0000 2.5000 C 26.5938 2.5000 25.1875 2.9453 23.7344 3.7657 L 7.2110 13.1875 C 6.6954 13.4688 6.2735 13.75 5.9219 14.1016 Z M 26.4063 53.5 L 26.4063 29.4532 L 4.3985 16.8906 C 4.2813 17.4063 4.2110 17.9688 4.2110 18.6719 L 4.2110 36.9297 C 4.2110 40.3281 5.4063 41.5938 7.5860 42.8360 L 25.9375 53.2891 C 26.1016 53.3828 26.2422 53.4532 26.4063 53.5 Z M 29.5938 53.5 C 29.7579 53.4532 29.8985 53.3828 30.0626 53.2891 L 48.4141 42.8360 C 50.5938 41.5938 51.7890 40.3281 51.7890 36.9297 L 51.7890 18.6719 C 51.7890 17.9688 51.7189 17.4063 51.6018 16.8906 L 29.5938 29.4532 Z'})

main_div = ET.Element('div', {'class': 'juce-modules-continer'})

toc_div = ET.SubElement(main_div, 'div', {'class': 'juce-module-toc-container'})
ET.SubElement(toc_div, 'p', {'class': 'juce-module-toc-desc'}).text = "Here are the JUCE modules with some brief descriptions:"
toc_table = ET.SubElement(toc_div, 'table', {'class': 'juce-module-toc-table'})
for module in modules:
    toc_row = ET.SubElement(toc_table, 'tr', {'class': 'juce-module-toc-row'})
    module_toc_name = ET.SubElement(toc_row, 'td', {'class': 'juce-module-toc-module-name'})
    ET.SubElement(module_toc_name, 'a', {'href': f'#{module['name']}', 'class': 'juce-module-toc-module-name-link'}).text = module['name']
    ET.SubElement(toc_row, 'td', {'class': 'juce-module-toc-module-decs'}).text = module_descriptions[module['name']]
ET.SubElement(main_div, 'div', {'class': 'juce-module-toc-divider'})

for module in modules:
    module_div = ET.SubElement(main_div, 'div', {'class': 'juce-module'})
    module_header_div = ET.SubElement(module_div, 'div', {'class': 'juce-module-header'}) 
    module_title_div = ET.SubElement(module_header_div, 'div', {'class': 'juce-module-title'}) 
    module_title_div.append(module_icon)
    ET.SubElement(module_title_div, 'span', {'id': module['name'], 'class': 'juce-module-name'}).text = module['name']
    ET.SubElement(module_header_div, 'span', {'class': 'juce-module-desc'}).text = module_descriptions[module['name']]
    module_contents_div = ET.SubElement(module_div, 'div', {'class': 'juce-module-contents'}) 
    write_module_html_recursive(module_contents_div, module)

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
