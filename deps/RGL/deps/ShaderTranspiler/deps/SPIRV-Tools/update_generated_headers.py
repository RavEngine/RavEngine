from pathlib import Path
import os
import sys
import shutil

# cd to script dir
os.chdir(os.path.dirname(sys.argv[0]))

destdir = Path("include/generated")
destdir.mkdir(exist_ok=True)

# copy headers
files = [
	"DebugInfo.h",
	"NonSemanticShaderDebugInfo100.h",
	"OpenCLDebugInfo100.h",
	"build-version.inc",
	"core.insts-unified1.inc",
	"debuginfo.insts.inc",
	"enum_string_mapping.inc",
	"extension_enum.inc",
	"generators.inc",
	"glsl.std.450.insts.inc",
	"nonsemantic.clspvreflection.insts.inc",
	"nonsemantic.shader.debuginfo.100.insts.inc",
	"opencl.debuginfo.100.insts.inc",
	"opencl.std.insts.inc",
	"operand.kinds-unified1.inc",
	"spv-amd-gcn-shader.insts.inc",
	"spv-amd-shader-ballot.insts.inc",
	"spv-amd-shader-explicit-vertex-parameter.insts.inc",
	"spv-amd-shader-trinary-minmax.insts.inc"
]
for file in files:
	shutil.copy(Path("build") / file, destdir)