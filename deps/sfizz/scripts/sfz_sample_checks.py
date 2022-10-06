#!/usr/bin/python3
import re
import os
import argparse

rex = r"sample=([a-zA-Z0-9-_#.&\/\s\\\(\),\*]+?)\s*(?=[a-zA-Z0-9_]*=|//.*|$|<[a-z]+>)"


parser = argparse.ArgumentParser(description="Simple script to update the sample names in an existing SFZ files")
parser.add_argument('file', type=str, help="SFZ input file")
parser.add_argument('--output', type=str, help="Output file name; if not specified, _corrected will be appended to the input")
parser.add_argument('--test', action='store_true', help="Test run")
args = parser.parse_args()

assert os.path.exists(args.file), "File does not exists"

root_directory = os.path.dirname(args.file) + '/'

def case_insensitive_check(filename):
	elements = filename.split('/')
	path = ''
	for e in elements:
		if path != '':
			path += '/'
		found = False

		if os.path.exists(root_directory + path + e):
			path += e
			continue

		for dir_entry in os.listdir(root_directory + path):
			if dir_entry.lower() == e.lower():
				path += dir_entry
				found = True

		if found == False:
			return None

	return path

with open(args.file, 'r') as file:
	content = file.read()

output_content = content
replacements = 0
for match in re.findall(rex, content):
	if not os.path.exists(root_directory + match):
		possible = case_insensitive_check(match)
		if possible is not None:
			print("{} -> {}".format(match, possible))
			replacements += 1
			output_content = re.sub(match, possible, output_content)
		else:
			print("{} -> ????".format(match))

if args.test:
	exit(0)

if replacements == 0:
	print("All samples seem to exist, not writing a file...")
	exit(0)

if args.output is None:
	output_file, ext = os.path.splitext(args.file)
	output_file += '_corrected'
	output_file += ext
else:
	output_file = args.output

print("Writing corrected file to", output_file)
with open(output_file, 'w') as file:
	file.write(output_content)
