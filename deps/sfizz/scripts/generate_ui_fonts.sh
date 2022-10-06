#!/bin/sh
set -e

if ! test -d "src"; then
    echo "Please run this in the project root directory."
    exit 1
fi

root="`pwd`"
fonts="$root/plugins/editor/resources/Fonts"

cd scripts/misc-icons/
./generate_icons_font.py
mv -f sfizz-misc-icons.ttf "$fonts"
cd ../../

if test ! -d plugins/editor/external/fluentui-system-icons; then
  cd plugins/editor/external
  git clone https://github.com/sfztools/fluentui-system-icons.git
  cd fluentui-system-icons
else
  cd plugins/editor/external/fluentui-system-icons
  git checkout master
  git pull origin master
fi

./generate_icons_font.py -s regular -w 20 -n 'Sfizz Fluent System R20' \
                         -o "$fonts/sfizz-fluentui-system-r20.ttf"
./generate_icons_font.py -s filled -w 20 -n 'Sfizz Fluent System F20' \
                         -o "$fonts/sfizz-fluentui-system-f20.ttf"
