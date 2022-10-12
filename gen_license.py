import os
import pathlib

# Recurse every directory looking for files containing "License" in the name
rootdir = os.path.dirname(os.path.realpath(__file__))
for root, subdirs, files in os.walk(rootdir):
    for file in files:
        suffix = pathlib.Path(file).suffix.lower()
        if any(x in suffix for x in [".py",".gpl",".lgpl"]) or "gpl" in file.lower() or "template" in file.lower():  #exclude this script or the optional GPL licenses
            continue
        if any(x in file.lower() for x in ["license","copying"]):
            filepath = os.path.join(root,file)
            
            print("===================================================================")
            print(os.path.relpath(filepath,rootdir))
            with open(filepath) as f:
               print(f.read())

