origin=`pwd`
cd ~/Library/Developer/Xcode/DerivedData/
mkdir -p "`basename ${PWD##}`_mac" && cd "$_"
cmake -G "Xcode" -Wno-dev "$origin"
open ShaderTranspiler.xcodeproj