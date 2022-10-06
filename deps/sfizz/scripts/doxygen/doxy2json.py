#!/usr/bin/env python3
"""
    SPDX-License-Identifier: BSD-2-Clause

    This code is part of the sfizz library and is licensed under a BSD 2-clause
    license. You should have receive a LICENSE.md file along with the code.
    If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

    Converts the Doxygen XML output to a custom JSON structure.

    The JSON output will be used on a Jekyll website,
    parsed by a related layout (_layouts/doxygen.html).

    Known bugs / wish list:

    - Replace `strip_tags_in_text()` and `tag_list_as_string()` with a recursive
      parsing function able to extract also detailed descriptions paragraphs
      (E.g. `sfizz_oversampling_factor_t` has 3), getting rid of various
      `@since`, `@note` and `@return`, which are parsed separately.
      It should also covert the Doxygen custom tags (like <bold>, see below,
      or should this be done by some XSLT trasformation file?).

    - Tags in ALIASES list needs to be escaped (parsed by tag_list_as_string())
      to avoid Doxygen convert them in its own tag structure, by loosing some
      details (e.g.: <b style=\"color:blue\">true</b> becomes <bold>true</bold>,
      use \<b\> as escaping and replace double quotes with apostrophe instead).

    - Some link references are lost like in
      `sfizz_export_midnam` -> `sfizz_free_memory`

    - Merge the work done previously to be able to fully automate the process
      to be used in various CIs.
"""
import xml.etree.ElementTree as ET
import json

# TODO: scan for files
tree = ET.parse('xml/classsfz_1_1_sfizz.xml')
root = tree.getroot()
data = {}

def strip_tags_in_text(element):
    if element is None:
        return ""

    returned_string = element.text or ""
    for t in element:
        if t.tag == "ref":
            returned_string += "<a href='#{}'>{}</a>".format(t.text.replace("()", ''), t.text)
            returned_string += t.tail or ""
        elif t.tag == "computeroutput":
            returned_string += "<code>{}</code>".format(t.text)
            returned_string += t.tail or ""
        else:
            returned_string += t.text or ""
            returned_string += t.tail or ""

    return returned_string.strip()

def tag_list_as_string(element_list):
    if element_list is None or len(element_list) == 0:
        return ""

    if element_list[0].text is not None:
        return element_list[0].text.strip()

    returned_string = ""
    for element in element_list:
        for t in element:
#           if t.tag == "bold": t.tag = 'b'
            returned_string += ET.tostring(t, encoding="unicode")

    return returned_string.strip()

name     = root.find("./compounddef/compoundname")
kind     = root.find("./compounddef").get("kind")
brief    = root.find("./compounddef/briefdescription/para")
include  = root.find("./compounddef/includes")
location = root.find("./compounddef/location").get("file")
language = root.find("./compounddef").get("language")
version  = root.get("version")

if name     is not None and name.text is not None:    data["name"]            = name.text
if kind     is not None:                              data["kind"]            = kind
if brief    is not None and brief.text is not None:   data["brief"]           = brief.text.strip()
if include  is not None and include.text is not None: data["include"]         = include.text.strip()
if location is not None:                              data["location"]        = location
if language is not None:                              data["language"]        = language
if version  is not None:                              data["doxygen_version"] = version
definitions = []

for sectiondef in root.iter("sectiondef"):

    def_kind   = sectiondef.get("kind")
    definition = {}
    members    = []
    definition["kind"] = def_kind

    for memberdef in sectiondef:

        member = {}
        member["name"]      = memberdef.find("name").text

        type_member         = memberdef.find("type")
        initializer_member  = memberdef.find("initializer")
        brief_member        = memberdef.find("briefdescription/para")
        description_member  = memberdef.find("detaileddescription/para")
        return_member       = memberdef.findall("detaileddescription/para/simplesect[@kind='return']/para")
        since_member        = memberdef.find("detaileddescription/para/simplesect[@kind='since']/para")
        note_member         = memberdef.find("detaileddescription/para/simplesect[@kind='note']/para")

        if type_member is not None:
            type_member = strip_tags_in_text(type_member)
            if type_member != '':
                member["type"] = type_member

        if initializer_member is not None and initializer_member.text is not None:
            member["initializer"] = initializer_member.text

        if brief_member is not None:
            member["brief"] = strip_tags_in_text(brief_member)

        if description_member is not None:
            description_member = strip_tags_in_text(description_member)
            if description_member != '':
                member["description"] = description_member

        if return_member is not None:
            return_member = tag_list_as_string(return_member)
            if return_member != '':
                member["return"] = return_member

        if since_member is not None and since_member.text is not None:
            member["since"] = since_member.text.strip()

        if note_member is not None and note_member.text is not None:
            member["note"] = strip_tags_in_text(note_member)

        members.append(member)

        if def_kind == "enum" or def_kind == "public-type":
            enumvalues = []
            for enumvalue in memberdef.iter("enumvalue"):
                enum = {}
                enum["name"] = enumvalue.find("name").text

                initializer_member = enumvalue.find("initializer")
                brief_member       = enumvalue.find("briefdescription/para")
                description_member = enumvalue.find("detaileddescription/para")

                if initializer_member is not None and initializer_member.text is not None:
                    enum["initializer"] = initializer_member.text

                if brief_member is not None:
                    enum["brief"] = strip_tags_in_text(brief_member)

                if description_member is not None:
                    enum["description"] = strip_tags_in_text(description_member)

                enumvalues.append(enum)

            member["values"] = enumvalues

        params = []
        for paramtag in memberdef.findall("param"):

            param = {}
            param["name"] = paramtag.find("declname").text
            param["type"] = strip_tags_in_text(paramtag.find("type"))

            param_items = memberdef.findall("detaileddescription/para/parameterlist[@kind='param']/parameteritem")
            for param_item in param_items:
                param_name = param_item.find("parameternamelist/parametername").text
                if param_name == param.get("name"):
                    description = param_item.find("parameterdescription/para")
                    if description is not None:
                        param["description"] = strip_tags_in_text(description)

            params.append(param)

        if params:
            member["params"] = params

    definition["members"] = members
    definitions.append(definition)

data["definitions"] = definitions

print(json.dumps(data, indent=2))
