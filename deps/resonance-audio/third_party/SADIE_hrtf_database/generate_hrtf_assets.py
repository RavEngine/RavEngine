"""
Copyright 2018 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS-IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

#!/usr/bin/python
"""Generates a C++ class that statically stores raw asset data.

Usage: generate_hrtf_assets.py <asset file> <output_path>

Parses an asset file and generates a compilable C++ class that statically
defines the content of all files declared in the asset file.

An asset file is an XML file that lists group of files. See below for an
example:

<?xml version="1.0"?>
<IAD name="Assets">
  <assets>
    <file>file1.txt</file>
    <file>file2.txt</file>
    <file name="dir/file1.txt">file1.txt</file>
    <file name="dir/file2.txt">file2.txt</file>
  </assets>
  <assets prefix="path">
    <file name="file1.txt">file1.txt</file>
    <file name="file2.txt">file2.txt</file>
  </assets>
</IAD>

Below that are any number of <assets> blocks which define groups of files with
similar paths. The filename to be added is the text of each <file> block, and
path to the file relative to the definition file. If an <asset> has a "prefix"
tag then that prefix is prepended to the filename.
"""

import array
import os
import re
import sys
import textwrap
from xml.etree import ElementTree


class Error(Exception):
  """Base class for errors."""


class InvalidAssetSyntaxError(Error):
  """Error representing an asset file with invalid syntax."""


#------------------------------------------------------------------------------
def BuildManifest(asset_file):
  """Builds a list of (filename, asset) pairs from an asset file.

  Args:
    asset_file: string - a file containing asset definitions.

  Returns:
    A tuple containing:
      - an asset class name
      - an list of (filename, asset) pairs, where filename is the absolute path
        to the file on local disk, and asset is the name the stored asset should
        be given.

  Raises:
    InvalidAssetSyntaxError: if asset_file has syntax errors.
    IOError: if asset_file or any files it references are not found.
  """
  if not os.path.exists(asset_file):
    raise IOError('Could not find asset file "%s"' % asset_file)

  # Load the xml.
  root = ElementTree.parse(asset_file).getroot()
  if root.tag != 'IAD':
    raise InvalidAssetSyntaxError('Root tag should be "IAD" not "%s"' %
                                  root.tag)
  if 'name' not in root.attrib:
    raise InvalidAssetSyntaxError('Root tag requires a "name" attribute')

  path_to_asset_file = os.path.abspath(os.path.dirname(asset_file))

  manifest = []
  for group in root.findall('assets'):
    # Get the path prefix if there is one.
    prefix = group.attrib.get('prefix', '')
    for asset in group.findall('file'):
      # See if the file exists.
      asset_filename = os.path.join(path_to_asset_file,
                                    os.path.normcase(asset.text))
      if not os.path.exists(asset_filename):
        raise IOError('Asset "%s" does not exist, searched in %s' %
                      (asset.text, path_to_asset_file))

      # The filename is either the asset text appended to the prefix, or a
      # requested name appended to the prefix.
      filename = os.path.join(prefix, asset.attrib.get('name', asset.text))
      # Add the full path to the file to the manifest.
      manifest += [(os.path.abspath(asset_filename), filename)]

  return root.attrib['name'], manifest


#------------------------------------------------------------------------------
def CamelCaseToUnderscore(name):
  """Converts a camel-case formatted string to an underscore format.

  Example: TestAssetClass -> test_asset_class
           HTTPServer -> http_server

  Args:
    name: string - the camel-case formatted string.

  Returns:
    The underscore formatted input string.
  """
  s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
  return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

#------------------------------------------------------------------------------


def GenerateAssetHeaderString(asset_class_name):
  """Generates C++ header of asset class.

  Args:
    asset_class_name: string - the asset class name.

  Returns:
    String of generated asset class header.
  """
  header_guard = asset_class_name.upper().replace(
      '/', '_').replace('.', '_') + '_'
  output_string = []
  output_string.append('#ifndef %s\n' % header_guard)
  output_string.append('#define %s\n\n' % header_guard)
  output_string.append('#include <memory>\n')
  output_string.append('#include <string>\n')
  output_string.append('#include <unordered_map>\n')
  output_string.append('#include <vector>\n\n')
  output_string.append('namespace sadie {\n\n')
  output_string.append(
      '// Note this class is automatically generated. Do not modify.\n')
  output_string.append('class %s {\n' % asset_class_name)
  output_string.append(' public:\n')
  output_string.append('  // Lookups and retrieves a file from an asset class.\n')
  output_string.append('  //\n')
  output_string.append('  // @param filename: Filename to be retrieved.\n')
  output_string.append('  // @return std::string with raw file data. In case of an error, a nullptr is\n')
  output_string.append('  //     returned. Caller must take over the ownership of the returned data.\n')
  output_string.append(
      '  std::unique_ptr<std::string> GetFile(const std::string& filename) '
      'const;\n\n')
  output_string.append(' private:\n')
  output_string.append(
      '  typedef std::unordered_map<std::string, std::vector<unsigned char>>\n')
  output_string.append('      AssetDataMap;\n')
  output_string.append('  static const AssetDataMap kAssetMap;\n')
  output_string.append('};\n\n')
  output_string.append('}  // namespace sadie\n\n')
  output_string.append('#endif  // %s\n' % header_guard)
  return ''.join(output_string)


#------------------------------------------------------------------------------
def GenerateMapEntryDataString(manifest_entry):
  """Generates formatted string of hexadecimal integers from a manifest entry.

  Generates a formatted string of comma-separated hexadecimal integers that
  define a C++ std::pair<std::string, std::vector<char>> map entry.

  Args:
    manifest_entry: a (filename, asset_name) pair, where filename is
      the absolute path to the file on local disk, and asset_name is the name
      the stored asset should be given.

  Returns:
    Formatted string containing asset name and binary file data:
    {"asset_name", {0x.., 0x..}}
  """
  with open(manifest_entry[0], 'rb') as f:
    file_data = f.read()
  ints = array.array('B', file_data)
  data_string = '{' + ', '.join('0x%x' % value for value in ints) + '}}'

  wrapper = textwrap.TextWrapper(initial_indent=' ' * 5,
                                 width=80,
                                 subsequent_indent=' ' * 6)
  wrapped_data_string = wrapper.fill(data_string)
  return '    {\"%s\",\n%s' % (manifest_entry[1], wrapped_data_string)


#------------------------------------------------------------------------------
def GenerateAssetImplementationString(manifest, asset_class_name,
                                      header_filename):
  """Generates C++ implementation of asset class.

  Args:
    manifest: list of lists of (filename, asset_name) pairs, where filename is
      the absolute path to the file on local disk, and asset_name is the name
      the stored asset should be given.
    asset_class_name: string - the asset class name.
    header_filename: string - header file name.

  Returns:
    String of generated asset class implementation.
  """
  output_string = []
  output_string.append('#include \"%s\"\n\n' %
                       header_filename.lower())
  output_string.append('namespace sadie {\n\n')
  output_string.append(
      'std::unique_ptr<std::string> %s::GetFile(\n' % asset_class_name)
  output_string.append('    const std::string& filename) const {\n')
  output_string.append('  AssetDataMap::const_iterator map_entry_itr = '
                       'kAssetMap.find(filename);\n')
  output_string.append('  if (map_entry_itr == kAssetMap.end()) {\n')
  output_string.append('    return nullptr;\n')
  output_string.append('  }\n')
  output_string.append('  const char* data =\n')
  output_string.append('      reinterpret_cast<const char*>('
                       'map_entry_itr->second.data());\n')
  output_string.append(
      '  const size_t data_size = map_entry_itr->second.size();\n')
  output_string.append('  return std::unique_ptr<std::string>('
                       'new std::string(data, data_size));\n')
  output_string.append('}\n\n')
  # Generate map definition.
  output_string.append('const %s::AssetDataMap %s::kAssetMap = {\n' %
                       (asset_class_name, asset_class_name))
  output_string.append(
      ',\n'.join(GenerateMapEntryDataString(f) for f in manifest))
  output_string.append('};\n\n')
  output_string.append('}  // namespace sadie\n')
  return ''.join(output_string)


#------------------------------------------------------------------------------
def main():
  if len(sys.argv) != 3:
    print 'Usage: %s <asset definition file> <output_path>' % sys.argv[0]
    return

  cur_dir = os.getcwd()
  asset_file_path = os.path.normcase(os.path.join(cur_dir, sys.argv[1]))
  output_dir_path = os.path.normcase(os.path.join(cur_dir, sys.argv[2]))

  # Parse manifest.
  classname, manifest = BuildManifest(asset_file_path)
  if not manifest:
    raise InvalidAssetSyntaxError(
        '"%s" does not contain any asset definitions' % asset_file_path)

  # Define class name, file names, output paths, etc.
  classname_underscore = CamelCaseToUnderscore(classname)
  header_filename = '%s.h' % classname_underscore
  implementation_filename = '%s.cc' % classname_underscore
  header_output_file_path = os.path.join(output_dir_path, header_filename)
  implementation_output_file_path = os.path.join(output_dir_path,
                                                 implementation_filename)

  # Render asset class and write output to .cc/.h files.
  header_content = GenerateAssetHeaderString(
      classname)
  cc_content = GenerateAssetImplementationString(
      manifest, classname, header_filename)

  with open(header_output_file_path, 'w') as text_file:
    text_file.write(header_content)
  with open(implementation_output_file_path, 'w') as text_file:
    text_file.write(cc_content)


if __name__ == '__main__':
  main()
