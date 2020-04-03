#!/bin/bash

# Copy required files
srcCPP="extras/pinplay/examples/pinplay-debugger-shell.cpp"
srcH="extras/pinplay/examples/pinplay-debugger-shell.H"
dest="extras/pinplay/sinuca_tracer/"
cp $srcCPP $srcH $dest

# Add necessary flags into the makefile
src="./source/tools/Config/makefile.unix.config"
temp="make.temp"
additionalFlags=" -Wno-deprecated-declarations -faligned-new"
sed -r 's/(TOOL_CXXFLAGS_NOOPT :=)(.*)/\1 '"$additionalFlags"' \2/g' $src > $temp
mv $temp $src