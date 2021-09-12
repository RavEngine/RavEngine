import os
import pathlib

# Recurse every directory looking for files containing "License" in the name
rootdir = os.path.dirname(os.path.realpath(__file__))
for root, subdirs, files in os.walk(rootdir):
    for file in files:
        if pathlib.Path(file).suffix == ".py":  #exclude this script
            continue
        if any(x in file.lower() for x in ["license","copying"]):
            filepath = os.path.join(root,file)
            
            print(os.path.relpath(filepath,rootdir))
            with open(filepath) as f:
               print(f.read())

