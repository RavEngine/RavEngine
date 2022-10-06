#!/bin/bash
set -e

svg_file="$1"
test -z "$svg_file" && exit 1

sizes="32 48 128 256"

rm -f "$svg_file".icon.*.png

for size in $sizes; do
  png_file="$svg_file".icon."$size".png
  inkscape -e "$png_file" "$svg_file" -w "$size" -h "$size"
  optipng "$png_file"
done

icotool -c -o "$svg_file".ico "$svg_file".icon.*.png
rm -f "$svg_file".icon.*.png
