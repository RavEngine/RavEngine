#!/usr/bin/fontforge

import fontforge
import optparse
import json
import os
import sys

CONFIG_FILE = os.path.splitext(__file__)[0] + '.json'
DATA = json.load(open(CONFIG_FILE))

def main():
    parser = optparse.OptionParser()
    parser.add_option("-o", dest="output",
                      help="set the output file",
                      metavar="output")
    parser.add_option("-n", dest="name",
                      help="set the font name",
                      metavar="name")

    (options, args) = parser.parse_args()

    output = options.output
    name = options.name

    if output is None:
        output = 'sfizz-misc-icons.ttf'
    if name is None:
        name = 'Sfizz Misc Icons'

    ###
    sys.stderr.write('* Font name: %s\n' % (name))
    sys.stderr.write('* Output to: %s\n' % (output))

    ###
    font = fontforge.font()
    font.familyname = name
    font.fullname = name
    font.descent = 0

    for (unicode, asset_name) in DATA['glyph_map'].items():
      unicode = int(unicode, 16)
      asset_path = 'icons/%s.svg' % (asset_name)

      glyph = font.createChar(unicode)
      glyph.importOutlines(asset_path)
      glyph.width = 800
      glyph.vwidth = 800

    #font.save('sfizz-misc-icons.sfd')
    font.generate(output)

if __name__ == '__main__':
    main()
